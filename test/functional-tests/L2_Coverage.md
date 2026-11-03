# Remote Debugger L2 Coverage Report

**Generated:** 2026-11-03  
**Component:** `Remote Debugger`  
**Source directory:** `remote_debugger/src`  
**Features:** `remote_debugger/test/functional-tests/features`  
**Tests:** `remote_debugger/test/functional-tests/tests`  
**Coverage tool:** lcov

## 1. Executive Summary

| Metric | Value |
|---|:---:|
| Feature files | 22 |
| Feature scenarios | 103 |
| Pytest files | 23 |
| Test functions (`test_*`, including skipped) | 118 |
| Disabled test functions (skip markers or commented definitions) | 0 |
| Files with no enabled test functions | 0 |
| Mapped feature/test pairs | 19 / 22 |
| Unmatched test files | 4 |
| Line coverage (lcov) | 49.7% (1508 of 3034 lines) |
| Source functions found | 128 |
| Functions hit | 76 |
| Functions not hit | 52 |
| Current functional coverage | 59.4% |

> Function coverage compares lcov FNDA counts with a heuristic scan of C function
> definitions under the source directory, including preprocessor branches.
> Test counts are a static inventory, not pass/fail results. Skip counts are
> estimates; conditional, class-level and module-level skips are not evaluated.

## 2. Feature-to-Test Mapping

| Feature File | Scenarios | Test File | Tests | Status |
|---|:---:|---|:---:|---|
| `rrd_append_dynamic_profile_static_not_found.feature` | 4 | - | 0 | Missing test file |
| `rrd_append_report.feature` | 4 | test_rrd_append_report.py | 7 | Count sufficient |
| `rrd_background_cmd_static_profile_report.feature` | 5 | test_rrd_background_cmd_static_profile_report.py | 5 | Count sufficient |
| `rrd_c_api_upload.feature` | 21 | test_rrd_c_api_upload.py | 5 | GAP (16 missing) |
| `rrd_corrupted_static_profile_report.feature` | 4 | test_rrd_corrupted_static_profile_report.py | 4 | Count sufficient |
| `rrd_debug_report_upload.feature` | 6 | test_rrd_debug_report_upload.py | 6 | Count sufficient |
| `rrd_deepsleep_static_report.feature` | 2 | test_rrd_deepsleep_static_report.py | 5 | Count sufficient |
| `rrd_dynamic_profile_missing_report.feature` | 4 | test_rrd_dynamic_profile_missing_report.py | 7 | Count sufficient |
| `rrd_dynamic_profile_node_length_exceeded.feature` | 4 | - | 0 | Missing test file |
| `rrd_dynamic_profile_report.feature` | 5 | test_rrd_dynamic_profile_report.py | 9 | Count sufficient |
| `rrd_dynamic_profile_subcategory_report.feature` | 5 | - | 0 | Missing test file |
| `rrd_empty_issuetype_event.feature` | 2 | test_rrd_empty_issuetype_event.py | 2 | Count sufficient |
| `rrd_harmful_command_static_report.feature` | 5 | test_rrd_harmful_command_static_report.py | 5 | Count sufficient |
| `rrd_single_instance.feature` | 1 | test_rrd_single_instance.py | 3 | Count sufficient |
| `rrd_start_control.feature` | 2 | test_rrd_start_control.py | 1 | GAP (1 missing) |
| `rrd_start_subscribe_and_wait.feature` | 1 | test_rrd_start_subscribe_and_wait.py | 4 | Count sufficient |
| `rrd_static_profile_category_report.feature` | 5 | test_rrd_static_profile_category_report.py | 5 | Count sufficient |
| `rrd_static_profile_missing_command_report.feature` | 5 | test_rrd_static_profile_missing_command_report.py | 5 | Count sufficient |
| `rrd_static_profile_report.feature` | 5 | test_rrd_static_profile_report.py | 5 | Count sufficient |
| `test_rrd_dynamic_profile_harmful_report.feature` | 5 | test_rrd_dynamic_profile_harmful_report.py | 7 | Count sufficient |
| `test_rrd_static_profile_report_with_suffix.feature` | 4 | test_rrd_static_profile_report_with_suffix.py | 5 | Count sufficient |
| `test_rrd_static_profile_report_with_suffix_negative_case.feature` | 4 | test_rrd_static_profile_report_with_suffix_negative_case.py | 5 | Count sufficient |

> Matching uses the same relative directory and feature stem, with an optional
> test_ prefix. Scenario Outline rows and parametrized cases are not expanded.

### Unmatched Test Files

| Test File | Tests |
|---|:---:|
| `test_rrd_append_dynamic_profile_static_notfound.py` | 7 |
| `test_rrd_dynamic_profile_rdm_node_length_exceeded.py` | 6 |
| `test_rrd_dynamic_subcategory_report.py` | 7 |
| `test_rrd_profile_data.py` | 3 |

## 3. Source Module Coverage (lcov)

| Module | Lines | Functions |
|---|:---:|:---:|
| `rrdCommandSanity.c` | 93.1% (67/72) | 100.0% (3/3) |
| `rrdDynamic.c` | 40.4% (59/146) | 40.0% (2/5) |
| `rrdEventProcess.c` | 68.8% (240/349) | 81.8% (9/11) |
| `rrdExecuteScript.c` | 94.1% (16/17) | 100.0% (2/2) |
| `rrdIarmEvents.c` | 8.6% (12/140) | 20.0% (1/5) |
| `rrdInterface.c` | 44.0% (189/430) | 63.6% (14/22) |
| `rrdJsonParser.c` | 81.8% (405/495) | 86.7% (13/15) |
| `rrdMain.c` | 60.3% (38/63) | 75.0% (3/4) |
| `rrdMsgPackDecoder.c` | 0.0% (0/280) | 0.0% (0/15) |
| `rrdRunCmdThread.c` | 55.2% (117/212) | 60.0% (6/10) |
| `rrd_archive.c` | 47.2% (126/267) | 72.7% (8/11) |
| `rrd_config.c` | 42.8% (95/222) | 66.7% (6/9) |
| `rrd_logproc.c` | 48.6% (36/74) | 75.0% (3/4) |
| `rrd_sysinfo.c` | 32.1% (36/112) | 33.3% (2/6) |
| `rrd_upload.c` | 40.7% (37/91) | 60.0% (3/5) |
| `uploadRRDLogs.c` | 54.7% (35/64) | 100.0% (1/1) |

> Only trace records matched to source files in this module contribute to line coverage.
> Ambiguous source paths are excluded; keep source-relative paths in the tracefile.

## 4. Scenario-to-Test Gap Analysis

| Feature File | Scenarios | Tests | Missing |
|---|:---:|:---:|:---:|
| `rrd_append_dynamic_profile_static_not_found.feature` | 4 | 0 | 4 |
| `rrd_c_api_upload.feature` | 21 | 5 | 16 |
| `rrd_dynamic_profile_node_length_exceeded.feature` | 4 | 0 | 4 |
| `rrd_dynamic_profile_subcategory_report.feature` | 5 | 0 | 5 |
| `rrd_start_control.feature` | 2 | 1 | 1 |

> Counts alone do not prove behavior coverage. Module-specific behavior analysis
> and recommendations belong in the module documentation.
