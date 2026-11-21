---
name: l2-coverage-auditor
description: 'Use when calculating, extending, fixing, or reporting L2 coverage across source modules. Creates or updates lcov/gcov runners, CI workflows, parsers, reports, module inventories, and focused tests.'
argument-hint: 'Name the module(s), coverage metric, or report/collector to work on.'
---

# L2 Coverage Auditor

## Purpose

Calculate trustworthy L2 coverage across the requested modules and make the smallest necessary changes to coverage collection, parsing, reporting, or tests when existing tooling cannot calculate it correctly. This skill applies to the remote-debugger repository and should derive scope from its build and test configuration rather than a hardcoded module list.

## Workflow

1. **Establish scope and existing behavior.** Read the L2 runner, CI workflows, coverage generator/parser, current report, build source lists, conditional build flags, and relevant tests. Identify whether the user wants a report-only audit or implementation changes. Keep changes within coverage tooling, CI workflow, report generation, and tests unless application-source instrumentation is explicitly requested.
2. **Inventory built modules.** Use the authoritative build declarations (for example `Makefile.am`, `configure.ac`, and included make fragments) to enumerate sources and build conditions. Reconcile this inventory with the coverage trace, report generator, and tests. Do not silently omit conditional modules, nested sources, or modules with no trace data.
3. **Validate the run evidence.** Locate the lcov tracefile and determine when/how it was generated, which tests ran, whether they passed, what was instrumented, and what paths were filtered. A checked-in report is historical unless its trace and test results are available and current. If fresh data is absent, label results stale or unavailable; do not manufacture percentages.
4. **Calculate metrics with explicit denominators.**
   - Line coverage: aggregate hit lines divided by found lines (`LH / LF`); overall coverage is the sum of hits divided by the sum of found lines, not an average of module percentages.
   - Function coverage: prefer lcov's per-source function counters (`FNH / FNF`) for compiled/instrumented code when present. If the project intentionally measures a source-inventory denominator that includes conditional branches not compiled in this run, report that as a separate metric and explain the scanner and build-configuration caveats. Never combine counters with incompatible denominators.
   - Branch coverage: report `BRH / BRF` only when branch instrumentation and counters are present. Distinguish unavailable data from zero coverage.
   - Aggregate each trace record by its full source path. Detect duplicate records and basename collisions instead of silently overwriting one module with another.
5. **Keep other coverage signals separate.** Feature-to-test filename matches, scenario counts, test-function counts, and behavior tables are inventory/traceability signals, not runtime coverage. Verify behavior claims against concrete tests and source paths. Use test execution results for pass/fail; test existence alone is not evidence that it ran successfully.
6. **Implement required changes.** When the user requests calculation changes or the audit finds a tooling gap, create or update only the necessary coverage runner, CI workflow (for example under `.github/workflows/`), parser/generator, module discovery, report, and focused tests. Prefer build-derived module discovery and lcov counters over maintaining duplicate hardcoded source lists. Preserve existing CLI/API behavior unless a change is required, and document changed metric definitions and compatibility implications.
7. **Validate the change.** Run focused parser/generator tests or add a small synthetic lcov fixture that covers multiple modules, missing records, duplicate records/path handling, zero denominators, and branch/function counters as applicable. Run formatting/type/lint checks for touched tooling. Do not run a full L2 suite unless requested and the intended container/environment is confirmed.
8. **Report results.** Summarize changed files, exact formulas/denominators, per-module results, missing or conditionally excluded modules, test evidence, and any limitations. Link relevant workspace files.

## Remote Debugger References

- L2 collection: `run_l2.sh` and any runner it invokes.
- Current report generator: `test/functional-tests/generate_l2_coverage_report.py`.
- Build/module inventory: `src/Makefile.am`, top-level `Makefile.am`, and `configure.ac`.
- L2 features/tests: `test/functional-tests/features/` and `test/functional-tests/tests/`.
- Existing generated report: `test/functional-tests/L2_Coverage.md`.

In this repository, the runner captures an initial lcov baseline, runs L2 pytest files, captures and merges test counters, filters `/usr/*`, and emits `/tmp/l2_coverage/coverage.info`. The current report generator also uses a C source scanner and filename/count heuristics; verify those behaviors before relying on them. The runner performs environment changes and cleanup, so do not execute it on the host or without explicit user authorization and a suitable isolated test environment.

## Required Report Sections

- Evidence: tracefile path/date, runner and test scope, test results, filters, and freshness.
- Measured overall and per-module line/function/branch counters with numerator and denominator.
- Modules missing trace records, build-conditional modules, and instrumentation gaps.
- Feature/test inventory and behavior traceability, explicitly labeled as separate from measured runtime coverage.
- Changes made (including generated runner/workflow/report files) and focused validation results, or the exact blocker preventing fresh calculation.
- Refreshed coverage details only when backed by a tracefile from the relevant test run; otherwise clearly mark report values as unavailable or historical.
