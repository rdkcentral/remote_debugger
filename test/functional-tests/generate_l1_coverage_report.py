#!/usr/bin/env python3
"""Generate an L1 unit-test coverage report from lcov and Google Test JSON."""

import argparse
import datetime
import json
import os


def parse_tracefile(tracefile, source_dir):
    source_names = {
        name for name in os.listdir(source_dir)
        if name.endswith('.c') and os.path.isfile(os.path.join(source_dir, name))
    }
    records = {}
    current = None

    with open(tracefile, encoding='utf-8') as coverage_file:
        for raw_line in coverage_file:
            line = raw_line.rstrip('\n')
            if line.startswith('SF:'):
                source_name = os.path.basename(line[3:])
                current = {
                    'name': source_name,
                    'lines_hit': 0,
                    'lines_found': 0,
                    'functions_hit': 0,
                    'functions_found': 0,
                    'branches_hit': 0,
                    'branches_found': 0,
                    'functions': {},
                }
            elif line == 'end_of_record' and current:
                if current['name'] in source_names:
                    records[current['name']] = current
                current = None
            elif current:
                if line.startswith('LH:'):
                    current['lines_hit'] = int(line[3:])
                elif line.startswith('LF:'):
                    current['lines_found'] = int(line[3:])
                elif line.startswith('FNH:'):
                    current['functions_hit'] = int(line[4:])
                elif line.startswith('FNF:'):
                    current['functions_found'] = int(line[4:])
                elif line.startswith('BRH:'):
                    current['branches_hit'] = int(line[4:])
                elif line.startswith('BRF:'):
                    current['branches_found'] = int(line[4:])
                elif line.startswith('FNDA:'):
                    count, name = line[5:].split(',', 1)
                    current['functions'][name] = int(count)

    return records


def parse_gtest_json(json_path):
    with open(json_path, encoding='utf-8') as results_file:
        root = json.load(results_file)

    suites = [{
        'name': suite.get('name', 'unnamed'),
        'tests': int(suite.get('tests', 0)),
        'failures': int(suite.get('failures', 0)),
        'disabled': int(suite.get('disabled', 0)),
        'skipped': int(suite.get('skipped', 0)),
    } for suite in root.get('testsuites', [])]

    return {
        'suites': suites,
        'tests': sum(suite['tests'] for suite in suites),
        'failures': sum(suite['failures'] for suite in suites),
        'disabled': sum(suite['disabled'] for suite in suites),
        'skipped': sum(suite['skipped'] for suite in suites),
    }


def percentage(hit, found):
    return 'N/A' if not found else f'{hit * 100 / found:.1f}%'


def render_report(records, test_results):
    totals = {
        key: sum(record[key] for record in records.values())
        for key in ('lines_hit', 'lines_found', 'functions_hit', 'functions_found',
                    'branches_hit', 'branches_found')
    }
    report = [
        '# Remote Debugger L1 Unit-Test Coverage',
        '',
        f'**Generated:** {datetime.date.today().isoformat()}  ',
        '**Coverage source:** lcov/gcov from the L1 Google Test run  ',
        '**Scope:** RRD C source files compiled into `remotedebugger_gtest`',
        '',
        '## Coverage Summary',
        '',
        '| Metric | Coverage | Hit / total |',
        '|---|:---:|:---:|',
        f'| Lines | **{percentage(totals["lines_hit"], totals["lines_found"])}** | {totals["lines_hit"]} / {totals["lines_found"]} |',
        f'| Functions | **{percentage(totals["functions_hit"], totals["functions_found"])}** | {totals["functions_hit"]} / {totals["functions_found"]} |',
        f'| Branches | **{percentage(totals["branches_hit"], totals["branches_found"])}** | {totals["branches_hit"]} / {totals["branches_found"]} |',
        '',
        '## L1 Test Results',
        '',
        '| Metric | Count |',
        '|---|---:|',
        f'| Test suites | {len(test_results["suites"])} |',
        f'| Test cases | {test_results["tests"]} |',
        f'| Failures | {test_results["failures"]} |',
        f'| Disabled | {test_results["disabled"]} |',
        f'| Skipped | {test_results["skipped"]} |',
        '',
        '## Per-Module Coverage',
        '',
        '| Source module | Lines | Functions | Branches |',
        '|---|:---:|:---:|:---:|',
    ]

    for name in sorted(records):
        record = records[name]
        report.append(
            f'| `{name}` '
            f'| {percentage(record["lines_hit"], record["lines_found"])} '
            f'({record["lines_hit"]}/{record["lines_found"]}) '
            f'| {percentage(record["functions_hit"], record["functions_found"])} '
            f'({record["functions_hit"]}/{record["functions_found"]}) '
            f'| {percentage(record["branches_hit"], record["branches_found"])} '
            f'({record["branches_hit"]}/{record["branches_found"]}) |'
        )

    report.extend(['', '## Test Suites', '', '| Suite | Tests | Failures | Disabled | Skipped |',
                    '|---|---:|---:|---:|---:|'])
    for suite in test_results['suites']:
        report.append(
            f'| `{suite["name"]}` | {suite["tests"]} | {suite["failures"]} '
            f'| {suite["disabled"]} | {suite["skipped"]} |'
        )

    uncovered = [
        (record['name'], name)
        for record in records.values()
        for name, count in record['functions'].items()
        if count == 0
    ]
    report.extend(['', '## Uncovered Functions', ''])
    if uncovered:
        report.extend([
            'These functions were present in the instrumented RRD source but had no recorded call during this L1 run.',
            '',
            '| Source module | Function |',
            '|---|---|',
        ])
        report.extend(f'| `{module}` | `{name}` |' for module, name in sorted(uncovered))
    else:
        report.append('All instrumented RRD functions were called at least once.')

    report.extend([
        '',
        '## Coverage Follow-Up',
        '',
        'Use the lowest-coverage modules and uncovered-function list to prioritize focused unit tests. '
        'Coverage indicates execution, not assertion quality or complete behavior validation.',
        '',
    ])
    return '\n'.join(report)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--tracefile', '--lcov-file', dest='tracefile', required=True)
    parser.add_argument('--gtest-json', required=True)
    parser.add_argument('--source-dir', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()

    records = parse_tracefile(args.tracefile, args.source_dir)
    if not records:
        raise SystemExit(f'No RRD C source coverage records found in {args.tracefile}')
    test_results = parse_gtest_json(args.gtest_json)
    output_dir = os.path.dirname(os.path.abspath(args.output))
    os.makedirs(output_dir, exist_ok=True)
    with open(args.output, 'w', encoding='utf-8') as report_file:
        report_file.write(render_report(records, test_results))
    print(f'Generated {args.output} from {len(records)} RRD C source files')


if __name__ == '__main__':
    main()
