#!/usr/bin/env python3
"""Generate a module-neutral L2 coverage Markdown report."""

import argparse
import datetime
import json
import os
import re


_C_KEYWORDS = {'if', 'while', 'for', 'switch', 'else', 'return', 'sizeof', 'do'}


def percentage(hit, total):
    return 'N/A' if not total else f'{hit / total * 100:.1f}%'


def parse_lcov(path):
    records = {}
    current = None
    with open(path, encoding='utf-8', errors='replace') as trace:
        for raw in trace:
            line = raw.rstrip()
            if line.startswith('SF:'):
                name = os.path.basename(line[3:])
                current = {'lines_hit': 0, 'lines_total': 0, 'functions_hit': 0,
                           'functions_total': 0, 'hits': set()}
            elif line == 'end_of_record' and current is not None:
                records[name] = current
                current = None
            elif current is not None:
                if line.startswith('LH:'):
                    current['lines_hit'] = int(line[3:])
                elif line.startswith('LF:'):
                    current['lines_total'] = int(line[3:])
                elif line.startswith('FNH:'):
                    current['functions_hit'] = int(line[4:])
                elif line.startswith('FNF:'):
                    current['functions_total'] = int(line[4:])
                elif line.startswith('FNDA:'):
                    count, _, function = line[5:].partition(',')
                    if int(count) > 0:
                        current['hits'].add(function)

    line_hit = sum(record['lines_hit'] for record in records.values())
    line_total = sum(record['lines_total'] for record in records.values())
    return records, {
        'lines_hit': line_hit,
        'lines_total': line_total,
        'lines_pct': percentage(line_hit, line_total),
    }


def strip_c_comments_and_literals(source):
    output = []
    index = 0
    while index < len(source):
        if source.startswith('//', index):
            newline = source.find('\n', index)
            index = len(source) if newline < 0 else newline
        elif source.startswith('/*', index):
            close = source.find('*/', index + 2)
            index = len(source) if close < 0 else close + 2
            output.append(' ')
        elif source[index] in '"\'':
            quote = source[index]
            cursor = index + 1
            while cursor < len(source) and source[cursor] != quote:
                cursor += 2 if source[cursor] == '\\' else 1
            index = cursor + 1
            output.append('""')
        else:
            output.append(source[index])
            index += 1
    return ''.join(output)


def source_function_names(path):
    """Return unique function names from a C file, including all preprocessor branches."""
    with open(path, encoding='utf-8', errors='replace') as source_file:
        source = strip_c_comments_and_literals(source_file.read())
    names, depth, declaration = set(), 0, ''
    for line in source.splitlines():
        if line.lstrip().startswith('#'):
            continue
        for char in line:
            if char == '{':
                if depth == 0:
                    match = re.search(r'([A-Za-z_]\w*)\s*\([^;{}]*\)\s*$', declaration)
                    if match and match.group(1) not in _C_KEYWORDS:
                        names.add(match.group(1))
                    declaration = ''
                depth += 1
            elif char == '}':
                depth -= 1
                if depth == 0:
                    declaration = ''
            elif depth == 0:
                declaration = '' if char == ';' else declaration + char
        if depth == 0:
            declaration += ' '
    return names


def scan_source(source_dir):
    return {
        filename: source_function_names(os.path.join(source_dir, filename))
        for filename in sorted(os.listdir(source_dir))
        if filename.endswith('.c') and os.path.isfile(os.path.join(source_dir, filename))
    }


def count_scenarios(path):
    with open(path, encoding='utf-8', errors='replace') as source_file:
        return sum(bool(re.match(r'\s*Scenario(?:\s+Outline)?:', line)) for line in source_file)


def scan_features(features_dir):
    return {
        filename: count_scenarios(os.path.join(features_dir, filename))
        for filename in sorted(os.listdir(features_dir)) if filename.endswith('.feature')
    }


def scan_tests(tests_dir):
    return {
        filename: len(re.findall(r'^def test_', open(os.path.join(tests_dir, filename), encoding='utf-8', errors='replace').read(), re.M))
        for filename in sorted(os.listdir(tests_dir))
        if filename.startswith('test_') and filename.endswith('.py')
    }


def load_testcase_list(path):
    with open(path, encoding='utf-8') as testcase_file:
        lines = testcase_file.readlines()
    header = '| Functional Testcases List |'
    try:
        first_row = next(index for index, line in enumerate(lines) if line.strip() == header) + 2
    except StopIteration as error:
        raise ValueError(f'Missing Functional Testcases List table in {path}') from error

    names = []
    for line in lines[first_row:]:
        row = line.strip()
        if not row.startswith('|'):
            if names:
                break
            continue
        cells = [cell.strip() for cell in row.strip('|').split('|')]
        if len(cells) != 1 or not cells[0]:
            raise ValueError(f'Expected one testcase per row: {row}')
        names.append(cells[0])
    if len(names) != len(set(names)):
        raise ValueError(f'Duplicate testcase names in {path}')
    return names


def load_module_profile(path):
    if not path:
        return {}
    with open(path, encoding='utf-8') as profile_file:
        profile = json.load(profile_file)
    assessments = profile.get('testcase_assessments', {})
    if not isinstance(assessments, dict):
        raise ValueError("Module profile 'testcase_assessments' must be an object")
    return assessments


def map_feature_tests(features, tests):
    pairs = []
    matched_tests = set()
    for feature, scenarios in features.items():
        stem = feature[:-len('.feature')]
        candidate = (stem if stem.startswith('test_') else f'test_{stem}') + '.py'
        test_count = tests.get(candidate)
        if test_count is not None:
            matched_tests.add(candidate)
        pairs.append((feature, scenarios, candidate if test_count is not None else None, test_count or 0))
    return pairs, sorted(set(tests) - matched_tests)


def render_report(module_name, tracefile, source_dir, features_dir, tests_dir,
                  testcase_list, output, profile_path=None):
    lcov, line_summary = parse_lcov(tracefile)
    source_functions = scan_source(source_dir)
    features = scan_features(features_dir)
    tests = scan_tests(tests_dir)
    testcase_names = load_testcase_list(testcase_list)
    assessments = load_module_profile(profile_path)
    for name, assessment in assessments.items():
        if name not in testcase_names:
            raise ValueError(f'Module profile testcase is missing from testcase list: {name}')
        if assessment.get('status', 'gap') not in {'covered', 'partial', 'gap'}:
            raise ValueError(f'Invalid testcase assessment status for {name}')
        for feature in assessment.get('features', []):
            if not os.path.isfile(os.path.join(features_dir, feature)):
                raise FileNotFoundError(f'{name}: feature evidence not found: {feature}')
        for test in assessment.get('tests', []):
            if not os.path.isfile(os.path.join(tests_dir, test)):
                raise FileNotFoundError(f'{name}: test evidence not found: {test}')

    source_total = sum(len(names) for names in source_functions.values())
    source_hit = sum(
        len(names & lcov.get(filename, {}).get('hits', set()))
        for filename, names in source_functions.items())
    source_missing = source_total - source_hit
    test_total = sum(tests.values())
    scenario_total = sum(features.values())
    testcase_total = len(testcase_names)
    testcase_counts = {
        status: sum(assessments.get(name, {}).get('status', 'gap') == status for name in testcase_names)
        for status in ('covered', 'partial', 'gap')
    }
    pairs, orphan_tests = map_feature_tests(features, tests)

    lines = [
        f'# {module_name} L2 Coverage Report', '',
        f'**Generated:** {datetime.date.today().isoformat()}  ',
        f'**Module:** `{module_name}`  ',
        '**Coverage source:** gcov/lcov trace plus module source, feature, test, and testcase-list files',
        '', '## 1. Executive Summary', '',
        '### L2 Test Summary', '',
        '| Metric | Value |', '|---|:---:|',
        f'| Feature files / scenarios | {len(features)} / {scenario_total} |',
        f'| Pytest files / test functions | {len(tests)} / {test_total} |',
        f'| Feature-to-test mapped pairs | {sum(pair[2] is not None for pair in pairs)} / {len(features)} features; {len(orphan_tests)} unmatched test files |',
        f'| Line coverage | {line_summary["lines_pct"]} ({line_summary["lines_hit"]} / {line_summary["lines_total"]} lines) |', '',
        '### L2 Test Coverage', '',
        '| Metric | Value |', '|---|:---:|',
        f'| Source functions found | {source_total} |',
        f'| Functions hit by L2 tests | {source_hit} |',
        f'| Functions not hit by L2 tests | {source_missing} |',
        f'| Function coverage | {percentage(source_hit, source_total)} |', '',
        '### Functional Test Coverage', '',
        '| Metric | Value |', '|---|:---:|',
        f'| Requested testcases | {testcase_total} |',
        f'| Fully covered testcases | {testcase_counts["covered"]} |',
        f'| Partially covered testcases | {testcase_counts["partial"]} |',
        f'| Uncovered testcases | {testcase_counts["gap"]} |',
        f'| Testcase coverage (full only) | {percentage(testcase_counts["covered"], testcase_total)} |',
        f'| Testcase coverage (full + partial evidence) | {percentage(testcase_counts["covered"] + testcase_counts["partial"], testcase_total)} |', '',
        '> Testcase status is module-profile metadata; `covered` does not assert the latest CI pass result.', '',
        '## 2. Feature and Test Mapping', '',
        '| Feature | Scenarios | Matching test file | Test functions |',
        '|---|:---:|---|:---:|',
    ]
    for feature, scenarios, test_file, count in pairs:
        lines.append(f'| `{feature}` | {scenarios} | `{test_file}` | {count} |')
    lines.extend(['', '### Test files without matching feature', ''])
    lines.extend(f'- `{name}`' for name in orphan_tests) if orphan_tests else lines.append('None.')
    lines.extend([
        '', '## 3. Functional Testcases', '',
        '| # | Testcase | Status | Feature files | Test files | Evidence note |',
        '|:---:|---|:---:|---|---|---|',
    ])
    for index, name in enumerate(testcase_names, 1):
        assessment = assessments.get(name, {})
        status = assessment.get('status', 'gap').upper()
        note = assessment.get('note', 'No module assessment provided.')
        features = ', '.join(f'`{item}`' for item in assessment.get('features', [])) or '—'
        tests = ', '.join(f'`{item}`' for item in assessment.get('tests', [])) or '—'
        lines.append(f'| {index} | {name} | {status} | {features} | {tests} | {note} |')
    lines.extend([
        '', '## 4. Source Function Coverage by File', '',
        '| Source file | Functions hit | Functions found | Coverage | Lines hit | Lines found |',
        '|---|:---:|:---:|:---:|:---:|:---:|',
    ])
    for filename in sorted(source_functions):
        names = source_functions[filename]
        record = lcov.get(filename, {})
        hit = len(names & record.get('hits', set()))
        lines.append(
            f'| `{filename}` | {hit} | {len(names)} | {percentage(hit, len(names))} '
            f'| {record.get("lines_hit", 0)} | {record.get("lines_total", 0)} |')
    os.makedirs(os.path.dirname(os.path.abspath(output)), exist_ok=True)
    with open(output, 'w', encoding='utf-8') as report:
        report.write('\n'.join(lines) + '\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--module-name', required=True)
    parser.add_argument('--tracefile', required=True)
    parser.add_argument('--source-dir', required=True)
    parser.add_argument('--features-dir', required=True)
    parser.add_argument('--tests-dir', required=True)
    parser.add_argument('--testcase-list', required=True)
    parser.add_argument('--output', required=True)
    parser.add_argument('--module-profile')
    args = parser.parse_args()
    render_report(args.module_name, args.tracefile, args.source_dir, args.features_dir,
                  args.tests_dir, args.testcase_list, args.output, args.module_profile)
    print(f'Written: {args.output}')


if __name__ == '__main__':
    main()
