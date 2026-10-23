#!/usr/bin/env python3
"""Generate an L1 unit-test coverage report from lcov and Google Test JSON."""

import argparse
import datetime
import json
import os


def parse_tracefile(tracefile, source_dir):
    source_dir = os.path.abspath(source_dir)
    root = os.path.dirname(source_dir)
    records = {}
    current = None

    with open(tracefile, encoding='utf-8') as coverage_file:
        for raw_line in coverage_file:
            line = raw_line.rstrip('\n')
            if line.startswith('SF:'):
                rel_path = os.path.relpath(os.path.abspath(line[3:]), root).replace(os.sep, '/')
                current = {
                    'name': os.path.basename(rel_path),
                    'path': rel_path,
                    'dir': os.path.dirname(rel_path),
                    'lines_hit': 0,
                    'lines_found': 0,
                    'functions_hit': 0,
                    'functions_found': 0,
                    'functions': {},
                }
            elif line == 'end_of_record' and current:
                if (not current['path'].startswith('..')
                        and os.path.isfile(os.path.join(root, current['path']))):
                    records[current['path']] = current
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


METRICS = ('lines_hit', 'lines_found', 'functions_hit', 'functions_found')


def sum_metrics(items):
    return {key: sum(item[key] for item in items) for key in METRICS}


def cell(hit, found):
    return f'{percentage(hit, found)} ({hit}/{found})'


def render_directory_section(records):
    by_dir = {}
    for record in records.values():
        by_dir.setdefault(record['dir'], []).append(record)

    report = [
        '## Coverage by Directory',
        '',
        'Expand a directory to see its per-file coverage.',
        '',
        '| Directory | Lines | Functions |',
        '|---|:---:|:---:|',
    ]
    for directory in sorted(by_dir):
        total = sum_metrics(by_dir[directory])
        report.append(
            f'| `{directory}` | {cell(total["lines_hit"], total["lines_found"])} '
            f'| {cell(total["functions_hit"], total["functions_found"])} |'
        )
    report.append('')

    for directory in sorted(by_dir):
        files = sorted(by_dir[directory], key=lambda record: record['name'])
        total = sum_metrics(files)
        report.extend([
            '<details>',
            f'<summary><code>{directory}</code> &mdash; lines '
            f'{percentage(total["lines_hit"], total["lines_found"])}, functions '
            f'{percentage(total["functions_hit"], total["functions_found"])} '
            f'({len(files)} files)</summary>',
            '',
            '| File | Lines | Functions |',
            '|---|:---:|:---:|',
        ])
        for record in files:
            report.append(
                f'| `{record["name"]}` '
                f'| {cell(record["lines_hit"], record["lines_found"])} '
                f'| {cell(record["functions_hit"], record["functions_found"])} |'
            )
        report.extend(['', '</details>', ''])
    return report


def render_report(records, test_results, source_rel):
    src_records = {path: rec for path, rec in records.items() if rec['dir'] == source_rel}
    totals = sum_metrics(records.values())
    report = [
        '# Remote Debugger L1 Unit-Test Coverage',
        '',
        f'**Generated:** {datetime.date.today().isoformat()}  ',
        '**Coverage source:** lcov/gcov from the L1 Google Test run  ',
        '**Scope:** All instrumented files in the lcov report (RRD sources plus unit-test code and mocks); '
        'the summary matches the overall lcov totals',
        '',
        '## Coverage Summary',
        '',
        '| Metric | Coverage | Hit / total |',
        '|---|:---:|:---:|',
        f'| Lines | **{percentage(totals["lines_hit"], totals["lines_found"])}** | {totals["lines_hit"]} / {totals["lines_found"]} |',
        f'| Functions | **{percentage(totals["functions_hit"], totals["functions_found"])}** | {totals["functions_hit"]} / {totals["functions_found"]} |',
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
        '| Source module | Lines | Functions |',
        '|---|:---:|:---:|',
    ]

    for path in sorted(src_records):
        record = src_records[path]
        report.append(
            f'| `{record["name"]}` '
            f'| {cell(record["lines_hit"], record["lines_found"])} '
            f'| {cell(record["functions_hit"], record["functions_found"])} |'
        )

    report.extend(['', *render_directory_section(records)])
    report.extend(['## Test Suites', '', '| Suite | Tests | Failures | Disabled | Skipped |',
                    '|---|---:|---:|---:|---:|'])
    for suite in test_results['suites']:
        report.append(
            f'| `{suite["name"]}` | {suite["tests"]} | {suite["failures"]} '
            f'| {suite["disabled"]} | {suite["skipped"]} |'
        )

    uncovered = [
        (record['name'], name)
        for record in src_records.values()
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
        report_file.write(render_report(
            records, test_results, os.path.basename(os.path.abspath(args.source_dir))))
    print(f'Generated {args.output} from {len(records)} RRD C source files')


if __name__ == '__main__':
    main()
