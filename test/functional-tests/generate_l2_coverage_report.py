#!/usr/bin/env python3
"""
Generate test/functional-tests/L2_Coverage.md.

Inputs
------
  --tracefile     lcov .info file produced by run_l2.sh
  --src-dir       directory containing the remotedebugger .c sources
  --features-dir  directory containing .feature files
  --tests-dir     directory containing test_*.py files
  --output        path for the generated markdown file

Usage
-----
  python3 generate_l2_coverage_report.py \
      --tracefile    /tmp/l2_coverage/coverage.info \
      --src-dir      src \
      --features-dir test/functional-tests/features \
      --tests-dir    test/functional-tests/tests \
      --output       test/functional-tests/L2_Coverage.md
"""

import argparse
import datetime
import os
import re


# ---------------------------------------------------------------------------
# lcov tracefile parser
# ---------------------------------------------------------------------------

def parse_tracefile(path):
    """Return (per_file_dict, summary_dict) from an lcov .info file."""
    files = {}
    cur = None
    with open(path) as fh:
        for raw in fh:
            line = raw.rstrip()
            if line.startswith('SF:'):
                name = os.path.basename(line[3:])
                cur = {'name': name, 'lh': 0, 'lf': 0,
                       'fnh': 0, 'fnf': 0, 'hit': set()}
            elif line == 'end_of_record' and cur:
                files[cur['name']] = cur
                cur = None
            elif cur:
                if   line.startswith('LH:'):  cur['lh']  = int(line[3:])
                elif line.startswith('LF:'):  cur['lf']  = int(line[3:])
                elif line.startswith('FNH:'): cur['fnh'] = int(line[4:])
                elif line.startswith('FNF:'): cur['fnf'] = int(line[4:])
                elif line.startswith('FNDA:'):
                    count, _, fname = line[5:].partition(',')
                    if int(count) > 0:
                        cur['hit'].add(fname)

    def tot(key): return sum(v[key] for v in files.values())
    summary = {
        'lines_pct':    _pct(tot('lh'), tot('lf')),
        'lines_det':    f"{tot('lh')} of {tot('lf')} lines",
    }
    return files, summary


def _pct(hit, found):
    if found == 0:
        return 'N/A'
    return f'{hit / found * 100:.1f}%'


def _bar(hit, found, width=20):
    if found == 0:
        return '░' * width
    filled = round(hit / found * width)
    return '█' * filled + '░' * (width - filled)


# ---------------------------------------------------------------------------
# source function scanner
# ---------------------------------------------------------------------------

_C_KEYWORDS = {'if', 'while', 'for', 'switch', 'else', 'return', 'sizeof', 'do'}


def _strip_c(code):
    out, i, n = [], 0, len(code)
    while i < n:
        c = code[i]
        if code.startswith('//', i):
            j = code.find('\n', i)
            i = n if j < 0 else j
        elif code.startswith('/*', i):
            j = code.find('*/', i + 2)
            i = n if j < 0 else j + 2
            out.append(' ')
        elif c in '"\'':
            j = i + 1
            while j < n and code[j] != c:
                j += 2 if code[j] == '\\' else 1
            i = j + 1
            out.append('""')
        else:
            out.append(c)
            i += 1
    return ''.join(out)


def _c_function_names(path):
    """Names of function definitions at brace depth 0; every #if branch counts."""
    with open(path, errors='replace') as fh:
        text = _strip_c(fh.read())
    names, depth, buf = set(), 0, ''
    for line in text.split('\n'):
        if line.strip().startswith('#'):
            continue
        for ch in line:
            if ch == '{':
                if depth == 0:
                    m = re.search(r'([A-Za-z_]\w*)\s*\([^;{}]*\)\s*$', buf)
                    if m and m.group(1) not in _C_KEYWORDS:
                        names.add(m.group(1))
                    buf = ''
                depth += 1
            elif ch == '}':
                depth -= 1
                if depth == 0:
                    buf = ''
            elif depth == 0:
                buf = '' if ch == ';' else buf + ch
        if depth == 0:
            buf += ' '
    return names


def scan_source_functions(src_dir):
    """Return {c_file: set(function names)} for every .c file directly in src_dir."""
    result = {}
    for fn in sorted(os.listdir(src_dir)):
        p = os.path.join(src_dir, fn)
        if fn.endswith('.c') and os.path.isfile(p):
            result[fn] = _c_function_names(p)
    return result


# ---------------------------------------------------------------------------
# test directory scanners
# ---------------------------------------------------------------------------

def scan_features(features_dir):
    """Return {filename: scenario_count}."""
    result = {}
    for fn in sorted(os.listdir(features_dir)):
        if fn.endswith('.feature'):
            result[fn] = _count_scenarios(os.path.join(features_dir, fn))
    return result


def scan_tests(tests_dir):
    """Return {filename: test_function_count}."""
    result = {}
    for fn in sorted(os.listdir(tests_dir)):
        if fn.startswith('test_') and fn.endswith('.py'):
            result[fn] = _count_test_funcs(os.path.join(tests_dir, fn))
    return result


def _count_scenarios(path):
    count = 0
    with open(path, errors='replace') as fh:
        for line in fh:
            if re.match(r'\s*Scenario(\s+Outline)?:', line):
                count += 1
    return count


def _count_test_funcs(path):
    count = 0
    with open(path, errors='replace') as fh:
        for line in fh:
            if re.match(r'^def test_', line):
                count += 1
    return count


def scan_disabled_tests(tests_dir):
    """Return (disabled_function_count, disabled_file_count).

    A test is disabled when its def is commented out or carries a skip marker;
    a file is disabled when it has no active test functions left.
    """
    funcs = files = 0
    for fn in sorted(os.listdir(tests_dir)):
        if not (fn.startswith('test_') and fn.endswith('.py')):
            continue
        with open(os.path.join(tests_dir, fn), errors='replace') as fh:
            text = fh.read()
        active = len(re.findall(r'^def test_', text, re.M))
        skipped = len(re.findall(
            r'^@pytest\.mark\.skip[^\n]*\n(?:@[^\n]*\n)*def test_', text, re.M))
        commented = len(re.findall(r'^\s*#\s*def test_', text, re.M))
        funcs += skipped + commented
        if active - skipped == 0 and (commented or skipped or active == 0):
            files += 1
    return funcs, files


# ---------------------------------------------------------------------------
# feature → test matching
# ---------------------------------------------------------------------------

def build_mapping(features, tests):
    """
    Return (pairs, orphan_features, orphan_tests).
    pairs = list of (feature_file, scenarios, test_file_or_None, test_funcs_or_0)
    """
    unmatched_tests = set(tests.keys())
    pairs = []
    for feat, scen in features.items():
        stem = feat.replace('.feature', '')
        candidate = (stem if stem.startswith('test_') else 'test_' + stem) + '.py'
        if candidate in tests:
            pairs.append((feat, scen, candidate, tests[candidate]))
            unmatched_tests.discard(candidate)
        else:
            pairs.append((feat, scen, None, 0))
    orphan_tests = {t: tests[t] for t in sorted(unmatched_tests)}
    orphan_feats = [p[0] for p in pairs if p[2] is None]
    return pairs, orphan_feats, orphan_tests


# ---------------------------------------------------------------------------
# static sections (priorities / recommendations — human judgment)
# ---------------------------------------------------------------------------

_RECOMMENDATIONS = """\
## 6. Gap Recommendations

### Priority 1 — Must Fix

| # | Gap | Impacted Modules |
|:---:|---|---|
| 1 | Implement missing `test_rrd_c_api_upload.py` scenarios (16 of 21 unimplemented) | `rrd_upload.c` |
| 2 | Add `rrd_profile_data.feature` for the existing `test_rrd_profile_data.py` | `rrdInterface.c` |
| 3 | Add WebCfg / MsgPack event L2 test (`rrd_webcfg_event.feature` + test) | `rrdMsgPackDecoder.c`, `rrdEventProcess.c` |

### Priority 2 — Should Fix

| # | Gap | Impacted Modules |
|:---:|---|---|
| 4 | Upload lock contention test | `rrd_upload.c` |
| 5 | Configuration fallback chain (RFC → DCM → dcm.properties) | `rrd_config.c` |
| 6 | Archive CPU throttle logic | `rrd_archive.c` |
| 7 | RDM download event with dynamic-profile cache miss | `rrdDynamic.c`, `rrdInterface.c` |
| 8 | Dynamic profile JSON parse failure | `rrdDynamic.c` |

### Priority 3 — Nice to Have

| # | Gap | Impacted Modules |
|:---:|---|---|
| 9  | RBUS registration / unregistration failure injection | `rrdInterface.c` |
| 10 | Message queue creation failure | `rrdMain.c` |
| 11 | Event thread creation failure | `rrdMain.c` |
| 12 | Directory creation / chdir failures | `rrdJsonParser.c` |
| 13 | `systemd-run` / `journalctl` execution failures | `rrdRunCmdThread.c` |
| 14 | Output file write errors | `rrdRunCmdThread.c` |
| 15 | Invalid deep sleep event type | `rrdDynamic.c` |
"""


def count_priorities(recommendations):
    """Return (high, medium, low) row counts from the P1/P2/P3 recommendation tables."""
    counts = []
    for part in re.split(r'^### Priority \d', recommendations, flags=re.M)[1:4]:
        counts.append(len(re.findall(r'^\|\s*\d+\s*\|', part, re.M)))
    return tuple(counts)


_BEHAVIOR_DETAIL = """\
## 4. Per-Behavior Coverage Detail

> Legend — **YES**: tested by an L2 scenario | **NO**: no test exists | **PARTIAL**: subset covered

### 4.1 Daemon Lifecycle

| Behavior | Covered |
|---|:---:|
| RBUS subscription + event wait | YES |
| RFC enable → daemon starts | YES |
| RFC disable → daemon stops | YES |
| Single instance enforcement | YES |
| Message queue creation failure | NO |
| Event thread creation failure | NO |
| Signal handling / graceful shutdown | NO |
| Device info file read failure | NO |

### 4.2 Static Profile Processing

| Behavior | Covered |
|---|:---:|
| Config file exists check | YES |
| IssueType event trigger + message flow | YES |
| JSON parse success + command execution | YES |
| Upload report success / failure | YES |
| Category-only issue type | YES |
| Suffixed issue type | YES |
| Overlength suffix (negative) | YES |
| Background command execution | YES |
| Missing command in profile | YES |
| Corrupted / invalid JSON profile | YES |

### 4.3 Dynamic Profile Processing

| Behavior | Covered |
|---|:---:|
| Dynamic profile fallback (static miss) | YES |
| Dynamic subcategory | YES |
| Dynamic profile missing → RDM trigger | YES |
| Append mode (static + dynamic) | YES |
| Append when static not found | YES |
| RDM download event (cache miss) | NO |
| Dynamic profile JSON parse failure | NO |

### 4.4 Harmful Command Detection

| Behavior | Covered |
|---|:---:|
| Static profile harmful command abort | YES |
| Dynamic profile harmful command abort | YES |
| Macro replacement edge cases | NO |
| Background command modification | PARTIAL |

### 4.5 Event Handling

| Behavior | Covered |
|---|:---:|
| IssueType RBUS event | YES |
| Empty IssueType event | YES |
| Deep sleep event | YES |
| WebCfg event (MsgPack decode) | NO |
| WebCfg corrupted data | NO |
| Multiple simultaneous IssueType events | NO |
| Invalid deep sleep event type | NO |

### 4.6 Upload & Archive

| Behavior | Covered |
|---|:---:|
| Upload via shell script | YES |
| Upload + download validation | YES |
| C API `rrd_upload_orchestrate` (happy path) | YES |
| C API NULL parameters | NO |
| C API empty / non-existent directory | NO |
| C API config loading / MAC retrieval | NO |
| C API archive creation + cleanup | NO |
| Concurrent upload lock | NO |
| Archive CPU throttle | NO |

### 4.7 Profile Data SET/GET (RBUS)

| Behavior | Covered |
|---|:---:|
| `setProfileData` / `getProfileData` via RBUS | PARTIAL (no feature file) |
| Profile category load error | NO |
| Profile file write error | NO |
"""

_SRC_MODULES = [
    'rrdMain.c', 'rrdInterface.c', 'rrdEventProcess.c', 'rrdJsonParser.c',
    'rrdRunCmdThread.c', 'rrdCommandSanity.c', 'rrdDynamic.c',
    'rrdExecuteScript.c', 'rrdMsgPackDecoder.c',
    'rrd_config.c', 'rrd_sysinfo.c', 'rrd_logproc.c',
    'rrd_archive.c', 'rrd_upload.c', 'rrdIarmEvents.c', 'uploadRRDLogs.c',
]


# ---------------------------------------------------------------------------
# markdown builder
# ---------------------------------------------------------------------------

def generate(pairs, orphan_feats, orphan_tests, features, files, summary, today,
             src_funcs, disabled_funcs, disabled_files):
    total_scenarios  = sum(p[1] for p in pairs)
    total_test_funcs = sum(p[3] for p in pairs) + sum(orphan_tests.values())
    total_features   = len(pairs)
    total_tests      = len(pairs) - len(orphan_feats) + len(orphan_tests)
    mapped           = sum(1 for p in pairs if p[2])
    gap_pairs        = [p for p in pairs if p[2] and p[3] < p[1]]

    # A source function is covered when lcov recorded at least one call to it.
    fn_total = sum(len(v) for v in src_funcs.values())
    fn_hit   = sum(len(src_funcs[m] & files[m]['hit']) for m in src_funcs if m in files)
    fn_miss  = fn_total - fn_hit
    high, medium, low = count_priorities(_RECOMMENDATIONS)

    lines = []

    # ── header ────────────────────────────────────────────────────────────────
    lines += [
        '# Remote Debugger L2 Coverage Report',
        '',
        f'**Generated:** {today}  ',
        '**Component:** `remotedebugger` (`src/`)  ',
        '**Test suite:** `test/functional-tests/`  ',
        '**Coverage tool:** lcov (source-level instrumentation via `--coverage`)',
        '',
        '---',
        '',
    ]

    # ── executive summary ─────────────────────────────────────────────────────
    lines += [
        '## 1. Executive Summary',
        '',
        '**L2 Test Summary**',
        '',
        '| Metric | Value |',
        '|---|:---:|',
        f'| Feature files | {total_features} |',
        f'| Feature scenarios | {total_scenarios} |',
        f'| Pytest files | {total_tests} |',
        f'| Test functions (`test_*`) | {total_test_funcs} |',
        f'| Mapped feature/test pairs | {mapped} / {total_features} |',
        f'| Unmatched test files | {len(orphan_tests)} |',
        f'| Line coverage (lcov) | {summary["lines_pct"]} ({summary["lines_det"]}) |',
        '',
        '**L2 Test Coverage**',
        '',
        '| Metric | Value |',
        '|---|:---:|',
        f'| Source functions found | {fn_total} |',
        f'| Functions hit | {fn_hit} |',
        f'| Functions not hit | {fn_miss} |',
        f'| Function coverage | {_pct(fn_hit, fn_total)} |',
        '',
        '> Function coverage = functions called at least once during the L2 run (lcov `FNDA` > 0),',
        '> divided by all function definitions in `src/*.c`, including those compiled out by `#if`.',
        '',
    ]

    # ── feature ↔ test mapping ────────────────────────────────────────────────
    lines += [
        '---',
        '',
        '## 2. Feature ↔ Test Mapping',
        '',
        '### 2.1 Mapped Pairs',
        '',
        '| # | Feature File | Scenarios | Test File | Tests | Status |',
        '|:---:|---|:---:|---|:---:|:---:|',
    ]
    idx = 1
    for feat, scen, test, tfuncs in pairs:
        if test is None:
            continue
        delta = tfuncs - scen
        status = 'PASS' if delta >= 0 else f'**GAP** ({abs(delta)} missing)'
        lines.append(f'| {idx} | `{feat}` | {scen} | `{test}` | {tfuncs} | {status} |')
        idx += 1

    total_scen_mapped  = sum(p[1] for p in pairs if p[2])
    total_funcs_mapped = sum(p[3] for p in pairs if p[2])
    lines += [
        f'| | **Totals** | **{total_scen_mapped}** | | **{total_funcs_mapped}** | |',
        '',
    ]

    over_pairs = [p for p in pairs if p[2] and p[3] > p[1]]
    if over_pairs:
        lines += [
            '### 2.2 Tests Exceeding Scenario Count',
            '',
            '> Extra functions handle setup/teardown split more granularly than feature scenarios.',
            '',
            '| Feature File | Scenarios | Test File | Tests | Extra |',
            '|---|:---:|---|:---:|:---:|',
        ]
        for feat, scen, test, tfuncs in over_pairs:
            lines.append(f'| `{feat}` | {scen} | `{test}` | {tfuncs} | +{tfuncs - scen} |')
        lines.append('')

    if orphan_tests:
        lines += [
            '### 2.3 Orphan Tests (test exists, no feature file)',
            '',
            '| Test File | Tests | Note |',
            '|---|:---:|---|',
        ]
        for tf, cnt in orphan_tests.items():
            lines.append(f'| `{tf}` | {cnt} | **Missing `.feature` file** |')
        lines.append('')

    if orphan_feats:
        lines += [
            '### 2.4 Orphan Features (feature exists, no test file)',
            '',
            '| Feature File | Scenarios | Note |',
            '|---|:---:|---|',
        ]
        for ff in orphan_feats:
            lines.append(f'| `{ff}` | {features[ff]} | **Missing test file** |')
        lines.append('')

    # ── per-module lcov coverage ───────────────────────────────────────────────
    lines += [
        '---',
        '',
        '## 3. Source Module Coverage (lcov)',
        '',
        '| Module | Lines | Functions | Coverage Bar |',
        '|---|:---:|:---:|---|',
    ]
    for mod in _SRC_MODULES:
        if mod in files:
            d = files[mod]
            if mod in src_funcs:
                f_tot, f_hit = len(src_funcs[mod]), len(src_funcs[mod] & d['hit'])
            else:
                f_tot, f_hit = d['fnf'], d['fnh']
            lines.append(
                f'| `{mod}` | {_pct(d["lh"], d["lf"])} ({d["lh"]}/{d["lf"]}) '
                f'| {_pct(f_hit, f_tot)} ({f_hit}/{f_tot}) '
                f'| `{_bar(d["lh"], d["lf"])}` |'
            )
        else:
            lines.append(f'| `{mod}` | — | — | no data |')
    lines.append('')

    # ── per-behavior detail (static) ──────────────────────────────────────────
    lines += ['---', '', _BEHAVIOR_DETAIL, '']

    # ── gap analysis ──────────────────────────────────────────────────────────
    lines += [
        '---',
        '',
        '## 5. Scenario-to-Test Gap Analysis',
        '',
    ]
    if gap_pairs:
        lines += [
            '| Feature File | Scenarios | Tests | Missing |',
            '|---|:---:|:---:|:---:|',
        ]
        for feat, scen, test, tfuncs in gap_pairs:
            lines.append(f'| `{feat}` | {scen} | {tfuncs} | **{scen - tfuncs}** |')
        lines.append('')
    else:
        lines += ['All mapped feature files have sufficient test function coverage.', '']

    if orphan_tests:
        lines += [
            '**Orphan tests** (no feature file — behavior is tested but not documented):',
            '',
        ]
        for tf in orphan_tests:
            lines.append(f'- `{tf}`')
        lines.append('')

    # ── recommendations (static) ─────────────────────────────────────────────
    lines += ['---', '', _RECOMMENDATIONS, '']

    # ── test infrastructure notes ────────────────────────────────────────────
    lines += [
        '---',
        '',
        '## 7. Test Infrastructure Notes',
        '',
        '| Interface | Tool | Used By |',
        '|---|---|---|',
        '| RBUS event trigger | `rbuscli set` | All event-based tests |',
        '| RBUS profile data | `rbuscli set/get` | `test_rrd_profile_data.py` |',
        '| Log scraping | `grep_rrdlogs()` in `helper_functions.py` | All tests |',
        '| Process control | `pidof`, `kill`, `nohup` | Setup / teardown |',
        '| Mock upload server | Mock xconf server | `test_rrd_debug_report_upload.py` |',
        '| Mock upload script | `uploadSTBLogs.sh` | Upload tests |',
        '',
        '- **Runner:** `pytest`, sequential per file',
        '- **Shared state:** tests within a file depend on prior test side effects',
        '- **Coverage run:** `kill_rrd()` sends `SIGTERM` when `RRD_COVERAGE_MODE` is set so gcov flushes',
        '',
    ]

    # ── appendix ──────────────────────────────────────────────────────────────
    lines += [
        '---',
        '',
        '## 8. Appendix: File Inventory',
        '',
        '### Feature Files',
        '',
        '| # | File | Scenarios |',
        '|:---:|---|:---:|',
    ]
    for i, (feat, scen, *_) in enumerate(pairs, 1):
        lines.append(f'| {i} | `{feat}` | {scen} |')
    lines.append(f'| | **Total** | **{total_scenarios}** |')
    lines.append('')

    lines += [
        '### Test Files',
        '',
        '| # | File | Tests |',
        '|:---:|---|:---:|',
    ]
    all_tests = [(p[2], p[3]) for p in pairs if p[2]] + list(orphan_tests.items())
    all_tests.sort()
    for i, (tf, cnt) in enumerate(all_tests, 1):
        lines.append(f'| {i} | `{tf}` | {cnt} |')
    lines.append(f'| | **Total** | **{total_test_funcs}** |')
    lines.append('')

    return '\n'.join(lines)


# ---------------------------------------------------------------------------
# entry point
# ---------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--tracefile',    required=True)
    ap.add_argument('--src-dir',      required=True)
    ap.add_argument('--features-dir', required=True)
    ap.add_argument('--tests-dir',    required=True)
    ap.add_argument('--output',       required=True)
    args = ap.parse_args()

    features = scan_features(args.features_dir)
    tests    = scan_tests(args.tests_dir)
    cov_files, summary = parse_tracefile(args.tracefile)
    pairs, orphan_feats, orphan_tests = build_mapping(features, tests)
    today = datetime.date.today().strftime('%Y-%m-%d')

    md = generate(pairs, orphan_feats, orphan_tests, features, cov_files, summary, today,
                  scan_source_functions(args.src_dir),
                  *scan_disabled_tests(args.tests_dir))

    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    with open(args.output, 'w', encoding='utf-8') as fh:
        fh.write(md)

    print(f'Written: {args.output}')


if __name__ == '__main__':
    main()
