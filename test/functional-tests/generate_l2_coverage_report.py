#!/usr/bin/env python3
"""
Generate test/functional-tests/L2_Coverage.md.

Inputs
------
  --tracefile   lcov .info file produced by run_l2.sh
  --features-dir  directory containing .feature files
  --tests-dir     directory containing test_*.py files
  --output        path for the generated markdown file

Usage
-----
  python3 generate_l2_coverage_report.py \
      --tracefile    /tmp/l2_coverage/coverage.info \
      --features-dir test/functional-tests/features \
      --tests-dir    test/functional-tests/tests \
      --output       test/functional-tests/L2_Coverage.md
"""

import argparse
import datetime
import os
import re
import sys


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
                       'fnh': 0, 'fnf': 0, 'brh': 0, 'brf': 0}
            elif line == 'end_of_record' and cur:
                files[cur['name']] = cur
                cur = None
            elif cur:
                if   line.startswith('LH:'):  cur['lh']  = int(line[3:])
                elif line.startswith('LF:'):  cur['lf']  = int(line[3:])
                elif line.startswith('FNH:'): cur['fnh'] = int(line[4:])
                elif line.startswith('FNF:'): cur['fnf'] = int(line[4:])
                elif line.startswith('BRH:'): cur['brh'] = int(line[4:])
                elif line.startswith('BRF:'): cur['brf'] = int(line[4:])

    def tot(key): return sum(v[key] for v in files.values())
    summary = {
        'lines_pct':    _pct(tot('lh'), tot('lf')),
        'lines_det':    f"{tot('lh')} of {tot('lf')} lines",
        'funcs_pct':    _pct(tot('fnh'), tot('fnf')),
        'funcs_det':    f"{tot('fnh')} of {tot('fnf')} functions",
        'branches_pct': _pct(tot('brh'), tot('brf')),
        'branches_det': f"{tot('brh')} of {tot('brf')} branches",
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
## 5. Gap Recommendations

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
"""


# ---------------------------------------------------------------------------
# markdown builder
# ---------------------------------------------------------------------------

def generate(pairs, orphan_feats, orphan_tests, files, summary, today):
    total_scenarios  = sum(p[1] for p in pairs)
    total_test_funcs = sum(p[3] for p in pairs) + sum(orphan_tests.values())
    total_features   = len(pairs)
    total_tests      = len(pairs) - len(orphan_feats) + len(orphan_tests)
    mapped           = sum(1 for p in pairs if p[2])
    gap_pairs        = [p for p in pairs if p[2] and p[3] < p[1]]

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
        '| Metric | Value |',
        '|---|:---:|',
        f'| Feature files | {total_features} |',
        f'| Feature scenarios | {total_scenarios} |',
        f'| Test files (pytest) | {total_tests} |',
        f'| Test functions (`test_*`) | {total_test_funcs} |',
        f'| Feature → Test mapped pairs | {mapped} / {total_features}'
        + (f' (+{len(orphan_tests)} orphan test{"s" if len(orphan_tests) != 1 else ""})' if orphan_tests else '') + ' |',
        f'| **Line coverage (lcov)** | **{summary["lines_pct"]}** ({summary["lines_det"]}) |',
        f'| **Branch coverage (lcov)** | **{summary["branches_pct"]}** ({summary["branches_det"]}) |',
        f'| **Function coverage (lcov)** | **{summary["funcs_pct"]}** ({summary["funcs_det"]}) |',
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
        '| # | Feature File | Scenarios | Test File | Tests | Gap |',
        '|:---:|---|:---:|---|:---:|:---:|',
    ]
    idx = 1
    for feat, scen, test, tfuncs in pairs:
        if test is None:
            continue
        delta = tfuncs - scen
        gap = '—' if delta >= 0 else f'**{abs(delta)} missing**'
        lines.append(f'| {idx} | `{feat}` | {scen} | `{test}` | {tfuncs} | {gap} |')
        idx += 1

    total_scen_mapped  = sum(p[1] for p in pairs if p[2])
    total_funcs_mapped = sum(p[3] for p in pairs if p[2])
    lines += [
        f'| | **Totals** | **{total_scen_mapped}** | | **{total_funcs_mapped}** | |',
        '',
    ]

    if orphan_tests:
        lines += [
            '### 2.2 Orphan Tests (test exists, no feature file)',
            '',
            '| Test File | Tests | Note |',
            '|---|:---:|---|',
        ]
        for tf, cnt in orphan_tests.items():
            lines.append(f'| `{tf}` | {cnt} | **Missing `.feature` file** |')
        lines.append('')

    if orphan_feats:
        lines += [
            '### 2.3 Orphan Features (feature exists, no test file)',
            '',
            '| Feature File | Scenarios | Note |',
            '|---|:---:|---|',
        ]
        for ff in orphan_feats:
            sc = features[ff]
            lines.append(f'| `{ff}` | {sc} | **Missing test file** |')
        lines.append('')

    # ── per-module lcov coverage ───────────────────────────────────────────────
    lines += [
        '---',
        '',
        '## 3. Source Module Coverage (lcov)',
        '',
        '| Module | Lines | Functions | Branches | Coverage Bar |',
        '|---|:---:|:---:|:---:|---|',
    ]
    src_modules = [
        'rrdMain.c', 'rrdInterface.c', 'rrdEventProcess.c', 'rrdJsonParser.c',
        'rrdRunCmdThread.c', 'rrdCommandSanity.c', 'rrdDynamic.c',
        'rrdExecuteScript.c', 'rrdMsgPackDecoder.c',
        'rrd_config.c', 'rrd_sysinfo.c', 'rrd_logproc.c',
        'rrd_archive.c', 'rrd_upload.c', 'rrdIarmEvents.c', 'uploadRRDLogs.c',
    ]
    for mod in src_modules:
        if mod in files:
            d = files[mod]
            lp = _pct(d['lh'], d['lf'])
            fp = _pct(d['fnh'], d['fnf'])
            bp = _pct(d['brh'], d['brf'])
            bar = _bar(d['lh'], d['lf'])
            lines.append(
                f'| `{mod}` | {lp} ({d["lh"]}/{d["lf"]}) '
                f'| {fp} ({d["fnh"]}/{d["fnf"]}) '
                f'| {bp} ({d["brh"]}/{d["brf"]}) '
                f'| `{bar}` |'
            )
        else:
            lines.append(f'| `{mod}` | — | — | — | no data |')
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

    # ── appendix ──────────────────────────────────────────────────────────────
    lines += [
        '---',
        '',
        '## 6. Appendix: File Inventory',
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
    all_tests = [(p[2], p[3]) for p in pairs if p[2]]
    all_tests += list(orphan_tests.items())
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
    ap.add_argument('--features-dir', required=True)
    ap.add_argument('--tests-dir',    required=True)
    ap.add_argument('--output',       required=True)
    args = ap.parse_args()

    global features  # used in build_mapping closure for orphan label
    features = scan_features(args.features_dir)
    tests    = scan_tests(args.tests_dir)
    cov_files, summary = parse_tracefile(args.tracefile)
    pairs, orphan_feats, orphan_tests = build_mapping(features, tests)
    today = datetime.date.today().strftime('%Y-%m-%d')

    md = generate(pairs, orphan_feats, orphan_tests, cov_files, summary, today)

    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    with open(args.output, 'w') as fh:
        fh.write(md)

    print(f'Written: {args.output}')
    print(f'  lines={summary["lines_pct"]}  '
          f'branches={summary["branches_pct"]}  '
          f'functions={summary["funcs_pct"]}')


if __name__ == '__main__':
    main()
