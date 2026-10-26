#!/usr/bin/env python3
"""
Generate test/functional-tests/L2_Coverage.md.

Inputs
------
  --tracefile     lcov .info file produced by run_l2.sh
  --src-dir       directory containing the remotedebugger .c sources
  --features-dir  directory containing .feature files
  --tests-dir     directory containing test_*.py files
    --testcase-list Markdown list of RRD testcases and evidence links
  --output        path for the generated markdown file

Usage
-----
  python3 generate_l2_coverage_report.py \
      --tracefile    /tmp/l2_coverage/coverage.info \
      --src-dir      src \
      --features-dir test/functional-tests/features \
      --tests-dir    test/functional-tests/tests \
            --testcase-list test/functional-tests/TestCase_Coverage.md \
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


def load_testcase_list(path, features_dir, tests_dir):
    """Parse the one-column testcase list and enrich entries with reviewed evidence."""
    with open(path, encoding='utf-8') as fh:
        lines = fh.readlines()

    header = '| Functional Testcases List |'
    try:
        table_start = next(i for i, line in enumerate(lines) if line.strip() == header) + 2
    except StopIteration as exc:
        raise ValueError(f'Missing Functional Testcases List table in {path}') from exc

    names = []
    for line in lines[table_start:]:
        line = line.strip()
        if not line.startswith('|'):
            if names:
                break
            continue
        cells = [cell.strip() for cell in line.strip('|').split('|')]
        if len(cells) != 1 or not cells[0]:
            raise ValueError(f'Expected one testcase name per row: {line}')
        names.append(cells[0])

    if not names:
        raise ValueError(f'No testcase rows found in {path}')

    seen = set()
    cases = []
    for name in names:
        if not name or name in seen:
            raise ValueError(f'Missing or duplicate testcase name: {name!r}')
        seen.add(name)
        assessment = _TESTCASE_ASSESSMENTS.get(name, {})
        case = {
            'name': name,
            'status': assessment.get('status', 'gap'),
            'features': assessment.get('features', []),
            'tests': assessment.get('tests', []),
            'note': assessment.get('note', 'New testcase; assess and add feature/test evidence.'),
        }
        for feature in case['features']:
            if not os.path.isfile(os.path.join(features_dir, feature)):
                raise FileNotFoundError(f'{name}: feature evidence not found: {feature}')
        for test in case['tests']:
            if not os.path.isfile(os.path.join(tests_dir, test)):
                raise FileNotFoundError(f'{name}: test evidence not found: {test}')
        cases.append(case)
    return cases


def testcase_coverage(cases):
    total = len(cases)
    covered = sum(case['status'] == 'covered' for case in cases)
    partial = sum(case['status'] == 'partial' for case in cases)
    gap = sum(case['status'] == 'gap' for case in cases)
    return {
        'total': total,
        'covered': covered,
        'partial': partial,
        'gap': gap,
        'strict_pct': _pct(covered, total),
        'evidence_pct': _pct(covered + partial, total),
    }


def _case_references(case, key):
    refs = case.get(key, [])
    return ', '.join(f'`{ref}`' for ref in refs) if refs else '—'


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

_TESTCASE_ASSESSMENTS = {
    'Static Profile Debug Report': {
        'status': 'covered', 'features': ['rrd_static_profile_report.feature'],
        'tests': ['test_rrd_static_profile_report.py'],
        'note': 'Static IssueType parsing, command execution, output, and upload invocation are asserted.',
    },
    'Enable the RRD via TR-181': {
        'status': 'covered', 'features': ['rrd_start_control.feature'],
        'tests': ['test_rrd_start_control.py'],
        'note': 'Enable parameter is queried and daemon startup is verified.',
    },
    'Disable the RRD via TR-181': {
        'status': 'covered', 'features': ['rrd_start_control.feature'],
        'tests': ['test_rrd_start_control.py'],
        'note': 'Disabled parameter behavior and daemon stop are verified.',
    },
    'Enable RRD via RFC and persist after reboot': {
        'note': 'No reboot-persistence scenario was found.',
    },
    'Validate RRD Debug Report update using RBUS and TR-181 commands': {
        'status': 'covered', 'features': ['rrd_static_profile_report.feature'],
        'tests': ['test_rrd_static_profile_report.py'],
        'note': 'The test sets IssueType with rbuscli and checks event delivery and report output.',
    },
    'Report upload after running the requested Issue command': {
        'status': 'covered', 'features': ['rrd_debug_report_upload.feature'],
        'tests': ['test_rrd_debug_report_upload.py'],
        'note': 'The test checks tarball creation and download from the mockxconf upload endpoint.',
    },
    'Dynamic Profile Debug Report': {
        'status': 'covered', 'features': ['rrd_dynamic_profile_report.feature'],
        'tests': ['test_rrd_dynamic_profile_report.py'],
        'note': 'Dynamic profile lookup, command execution, and report upload flow are exercised.',
    },
    'Append static commands from the dynamic profile for the same IssueType': {
        'status': 'covered', 'features': ['rrd_append_report.feature'],
        'tests': ['test_rrd_append_report.py'],
        'note': 'The test checks the merged static and dynamic command path.',
    },
    'Deep-sleep Profile Debug Report including WebConfig/IARM events': {
        'status': 'partial', 'features': ['rrd_deepsleep_static_report.feature'],
        'tests': ['test_rrd_deepsleep_static_report.py'],
        'note': 'The DEEPSLEEP profile report is tested; the WebConfig/IARM event path is not directly verified.',
    },
    'Set/Get category-level query from the static JSON profile': {
        'status': 'partial', 'tests': ['test_rrd_profile_data.py'],
        'note': 'The test sets a category and checks command success, but does not assert the returned category contents.',
    },
    'Set/Get all data from the static JSON profile': {
        'status': 'partial', 'tests': ['test_rrd_profile_data.py'],
        'note': 'The test requests all data but validates only the response type when output is non-empty.',
    },
    'Support Issue Debug Report requests with UUID': {
        'note': 'No UUID-bearing request scenario or assertion was found.',
    },
    'Support Issue Debug Report requests without UUID': {
        'note': 'No explicit UUID-absent request scenario was found.',
    },
    'Use the request UUID for report upload after command execution': {
        'note': 'No assertion connects a request UUID to the upload operation.',
    },
    'Confirm report upload to the S3 portal': {
        'status': 'partial', 'features': ['rrd_debug_report_upload.feature'],
        'tests': ['test_rrd_debug_report_upload.py'],
        'note': 'Upload and download are verified against mockxconf, not the S3 portal.',
    },
    'Match the request UUID with the uploaded report tarball': {
        'note': 'The tarball is checked for existence/download, but its UUID is not compared with the request.',
    },
    'Execute a valid static IssueType debug command': {
        'status': 'covered', 'features': ['rrd_static_profile_report.feature'],
        'tests': ['test_rrd_static_profile_report.py'],
        'note': 'The static profile test checks command validation, output creation, and service execution.',
    },
    'Verify all static IssueTypes': {
        'status': 'partial',
        'features': ['rrd_static_profile_report.feature', 'rrd_static_profile_category_report.feature'],
        'tests': ['test_rrd_static_profile_report.py', 'test_rrd_static_profile_category_report.py'],
        'note': 'Representative static IssueTypes are exercised; exhaustive profile coverage is not shown.',
    },
    'Attempt to execute a forbidden command': {
        'status': 'covered', 'features': ['rrd_harmful_command_static_report.feature'],
        'tests': ['test_rrd_harmful_command_static_report.py'],
        'note': 'The harmful-command test checks that command execution is aborted and upload skipped.',
    },
    'Send a malformed or empty command': {
        'status': 'partial', 'features': ['rrd_empty_issuetype_event.feature'],
        'tests': ['test_rrd_empty_issuetype_event.py'],
        'note': 'An empty IssueType is tested; malformed command input is not explicitly covered.',
    },
    'Download a dynamic debug profile via RDM': {
        'status': 'partial', 'features': ['rrd_dynamic_profile_missing_report.feature'],
        'tests': ['test_rrd_dynamic_profile_missing_report.py'],
        'note': 'The missing-profile path requests an RDM download; successful package delivery and installation are not verified.',
    },
    'Trigger a non-existent or misspelled IssueType': {
        'status': 'partial', 'features': ['rrd_dynamic_profile_missing_report.feature'],
        'tests': ['test_rrd_dynamic_profile_missing_report.py'],
        'note': 'Missing static/dynamic IssueType behavior is exercised, but misspellings are not isolated as a case.',
    },
    'Validate log/tarball creation and upload': {
        'status': 'covered', 'features': ['rrd_debug_report_upload.feature'],
        'tests': ['test_rrd_debug_report_upload.py'],
        'note': 'The feature and test check report archive creation, upload to mockxconf, and download.',
    },
    'Simulate S3 failure and check retry/logging': {
        'note': 'No S3 failure/retry scenario was found.',
    },
    'Try running sensitive system commands': {
        'status': 'partial', 'features': ['rrd_harmful_command_static_report.feature'],
        'tests': ['test_rrd_harmful_command_static_report.py'],
        'note': 'Harmful-command rejection is tested, but the requested sensitive-command set is not enumerated.',
    },
    'Send a debug command while in deep sleep': {
        'status': 'partial', 'features': ['rrd_deepsleep_static_report.feature'],
        'tests': ['test_rrd_deepsleep_static_report.py'],
        'note': 'DEEPSLEEP report processing is tested, but command timing during actual sleep is not asserted.',
    },
    'Validate resume on wake': {
        'note': 'No resume-on-wake assertion was found.',
    },
    'Flood remote debugger with commands (stress test)': {
        'note': 'No command-flood or stress test was found.',
    },
    'Delete/corrupt configuration and simulate network loss': {
        'status': 'partial', 'features': ['rrd_corrupted_static_profile_report.feature'],
        'tests': ['test_rrd_corrupted_static_profile_report.py'],
        'note': 'Corrupted static JSON is tested; network-loss behavior is not.',
    },
    'Run debugger alongside other system daemons': {
        'note': 'No multi-daemon coexistence test was found.',
    },
    'Firmware upgrade and disk stress': {
        'note': 'No firmware-upgrade or disk-stress scenario was found.',
    },
    'Set/Get with a misspelled category': {
        'status': 'partial', 'tests': ['test_rrd_profile_data.py'],
        'note': 'A non-existent category is tried, but the test does not assert the resulting profile contents.',
    },
    'Set/Get with an empty category': {
        'status': 'partial', 'tests': ['test_rrd_profile_data.py'],
        'note': 'An empty category is tried, but the test does not assert the resulting profile contents.',
    },
}


# ---------------------------------------------------------------------------
# markdown builder
# ---------------------------------------------------------------------------

def generate(pairs, orphan_feats, orphan_tests, features, files, summary, today,
             src_funcs, disabled_funcs, disabled_files, testcase_cases):
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
    testcase_summary = testcase_coverage(testcase_cases)

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
        '```',
        f'Total functions found in the module: {fn_total}',
        f'Functions covered by L2 tests (hit): {fn_hit}',
        f'Functions with no L2 coverage: {fn_miss}',
        '',
        f'Active L2 test functions: {total_test_funcs}',
        f'Disabled L2 test functions: {disabled_funcs}',
        f'Active feature scenarios: {total_scenarios}',
        f'Proposed new test scenarios: {high + medium + low}',
        '',
        f'High priority: {high}',
        f'Medium priority: {medium}',
        f'Low priority: {low}',
        '',
        f'Test files active: {total_tests - disabled_files}',
        f'Test files disabled (commented out): {disabled_files}',
        '',
        f'Estimated current L2 functional coverage: {_pct(fn_hit, fn_total)}',
        'Target L2 functional coverage: ~100%',
        '',
        f'Requested testcases: {testcase_summary["total"]}',
        f'Fully covered testcases: {testcase_summary["covered"]}',
        f'Partially covered testcases: {testcase_summary["partial"]}',
        f'Uncovered testcases: {testcase_summary["gap"]}',
        f'Testcase coverage (full only): {testcase_summary["strict_pct"]}',
        f'Testcase coverage (full + partial evidence): {testcase_summary["evidence_pct"]}',
        '```',
        '',
        '> Functional coverage = functions called at least once during the L2 run (lcov `FNDA` > 0)',
        '> divided by all function definitions in `src/*.c`, including those compiled out by `#if`.',
        '',
        '| Metric | Value |',
        '|---|:---:|',
        f'| Feature files | {total_features} |',
        f'| Feature scenarios | {total_scenarios} |',
        f'| Test files (pytest) | {total_tests} |',
        f'| Test functions (`test_*`) | {total_test_funcs} |',
        f'| Feature → Test mapped pairs | {mapped} / {total_features}'
        + (f' (+{len(orphan_tests)} orphan test{"s" if len(orphan_tests) != 1 else ""})'
           if orphan_tests else '') + ' |',
        f'| **Line coverage (lcov)** | **{summary["lines_pct"]}** ({summary["lines_det"]}) |',
        f'| **Function coverage (lcov)** | **{_pct(fn_hit, fn_total)}** ({fn_hit} of {fn_total} functions) |',
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

    lines += [
        '### 2.5 RRD Testcase Coverage',
        '',
        '> Status is an evidence review, not inferred from filename matching. A linked test is not marked covered unless its assertions verify the requested behavior.',
        '',
        '| # | RRD testcase | Coverage status | Feature files | Test files | Coverage notes |',
        '|:---:|---|:---:|---|---|---|',
    ]
    for index, case in enumerate(testcase_cases, 1):
        lines.append(
            f'| {index} | {case["name"]} | {case["status"].upper()} '
            f'| {_case_references(case, "features")} '
            f'| {_case_references(case, "tests")} '
            f'| {case.get("note", "")} |'
        )
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
    ap.add_argument('--testcase-list', required=True)
    ap.add_argument('--output',       required=True)
    args = ap.parse_args()

    features = scan_features(args.features_dir)
    tests    = scan_tests(args.tests_dir)
    testcase_cases = load_testcase_list(
        args.testcase_list, args.features_dir, args.tests_dir)
    cov_files, summary = parse_tracefile(args.tracefile)
    pairs, orphan_feats, orphan_tests = build_mapping(features, tests)
    today = datetime.date.today().strftime('%Y-%m-%d')

    md = generate(pairs, orphan_feats, orphan_tests, features, cov_files, summary, today,
                  scan_source_functions(args.src_dir),
                  *scan_disabled_tests(args.tests_dir), testcase_cases)

    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    with open(args.output, 'w', encoding='utf-8') as fh:
        fh.write(md)

    print(f'Written: {args.output}')


if __name__ == '__main__':
    main()
