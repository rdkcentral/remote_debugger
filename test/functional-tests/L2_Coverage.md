# Remote Debugger L2 Coverage Report

**Generated:** 2026-10-26  
**Component:** `remotedebugger` (`src/`)  
**Test suite:** `test/functional-tests/`  
**Coverage tool:** lcov (source-level instrumentation via `--coverage`)

---

## 1. Executive Summary

**L2 Test Summary**

```
Total functions found in the module: 128
Functions covered by L2 tests (hit): 76
Functions with no L2 coverage: 52

Active L2 test functions: 118
Disabled L2 test functions: 0
Active feature scenarios: 103
Proposed new test scenarios: 15

High priority: 3
Medium priority: 5
Low priority: 7

Test files active: 23
Test files disabled (commented out): 0

Estimated current L2 functional coverage: 59.4%
Target L2 functional coverage: ~100%

Requested testcases: 33
Fully covered testcases: 10
Partially covered testcases: 13
Uncovered testcases: 10
Testcase coverage (full only): 30.3%
Testcase coverage (full + partial evidence): 69.7%
```

> Functional coverage = functions called at least once during the L2 run (lcov `FNDA` > 0)
> divided by all function definitions in `src/*.c`, including those compiled out by `#if`.

| Metric | Value |
|---|:---:|
| Feature files | 22 |
| Feature scenarios | 103 |
| Test files (pytest) | 23 |
| Test functions (`test_*`) | 118 |
| Feature → Test mapped pairs | 19 / 22 (+4 orphan tests) |
| **Line coverage (lcov)** | **49.7%** (1508 of 3034 lines) |
| **Function coverage (lcov)** | **59.4%** (76 of 128 functions) |

---

## 2. Feature ↔ Test Mapping

### 2.1 Mapped Pairs

| # | Feature File | Scenarios | Test File | Tests | Status |
|:---:|---|:---:|---|:---:|:---:|
| 1 | `rrd_append_report.feature` | 4 | `test_rrd_append_report.py` | 7 | PASS |
| 2 | `rrd_background_cmd_static_profile_report.feature` | 5 | `test_rrd_background_cmd_static_profile_report.py` | 5 | PASS |
| 3 | `rrd_c_api_upload.feature` | 21 | `test_rrd_c_api_upload.py` | 5 | **GAP** (16 missing) |
| 4 | `rrd_corrupted_static_profile_report.feature` | 4 | `test_rrd_corrupted_static_profile_report.py` | 4 | PASS |
| 5 | `rrd_debug_report_upload.feature` | 6 | `test_rrd_debug_report_upload.py` | 6 | PASS |
| 6 | `rrd_deepsleep_static_report.feature` | 2 | `test_rrd_deepsleep_static_report.py` | 5 | PASS |
| 7 | `rrd_dynamic_profile_missing_report.feature` | 4 | `test_rrd_dynamic_profile_missing_report.py` | 7 | PASS |
| 8 | `rrd_dynamic_profile_report.feature` | 5 | `test_rrd_dynamic_profile_report.py` | 9 | PASS |
| 9 | `rrd_empty_issuetype_event.feature` | 2 | `test_rrd_empty_issuetype_event.py` | 2 | PASS |
| 10 | `rrd_harmful_command_static_report.feature` | 5 | `test_rrd_harmful_command_static_report.py` | 5 | PASS |
| 11 | `rrd_single_instance.feature` | 1 | `test_rrd_single_instance.py` | 3 | PASS |
| 12 | `rrd_start_control.feature` | 2 | `test_rrd_start_control.py` | 1 | **GAP** (1 missing) |
| 13 | `rrd_start_subscribe_and_wait.feature` | 1 | `test_rrd_start_subscribe_and_wait.py` | 4 | PASS |
| 14 | `rrd_static_profile_category_report.feature` | 5 | `test_rrd_static_profile_category_report.py` | 5 | PASS |
| 15 | `rrd_static_profile_missing_command_report.feature` | 5 | `test_rrd_static_profile_missing_command_report.py` | 5 | PASS |
| 16 | `rrd_static_profile_report.feature` | 5 | `test_rrd_static_profile_report.py` | 5 | PASS |
| 17 | `test_rrd_dynamic_profile_harmful_report.feature` | 5 | `test_rrd_dynamic_profile_harmful_report.py` | 7 | PASS |
| 18 | `test_rrd_static_profile_report_with_suffix.feature` | 4 | `test_rrd_static_profile_report_with_suffix.py` | 5 | PASS |
| 19 | `test_rrd_static_profile_report_with_suffix_negative_case.feature` | 4 | `test_rrd_static_profile_report_with_suffix_negative_case.py` | 5 | PASS |
| | **Totals** | **90** | | **95** | |

### 2.2 Tests Exceeding Scenario Count

> Extra functions handle setup/teardown split more granularly than feature scenarios.

| Feature File | Scenarios | Test File | Tests | Extra |
|---|:---:|---|:---:|:---:|
| `rrd_append_report.feature` | 4 | `test_rrd_append_report.py` | 7 | +3 |
| `rrd_deepsleep_static_report.feature` | 2 | `test_rrd_deepsleep_static_report.py` | 5 | +3 |
| `rrd_dynamic_profile_missing_report.feature` | 4 | `test_rrd_dynamic_profile_missing_report.py` | 7 | +3 |
| `rrd_dynamic_profile_report.feature` | 5 | `test_rrd_dynamic_profile_report.py` | 9 | +4 |
| `rrd_single_instance.feature` | 1 | `test_rrd_single_instance.py` | 3 | +2 |
| `rrd_start_subscribe_and_wait.feature` | 1 | `test_rrd_start_subscribe_and_wait.py` | 4 | +3 |
| `test_rrd_dynamic_profile_harmful_report.feature` | 5 | `test_rrd_dynamic_profile_harmful_report.py` | 7 | +2 |
| `test_rrd_static_profile_report_with_suffix.feature` | 4 | `test_rrd_static_profile_report_with_suffix.py` | 5 | +1 |
| `test_rrd_static_profile_report_with_suffix_negative_case.feature` | 4 | `test_rrd_static_profile_report_with_suffix_negative_case.py` | 5 | +1 |

### 2.3 Orphan Tests (test exists, no feature file)

| Test File | Tests | Note |
|---|:---:|---|
| `test_rrd_append_dynamic_profile_static_notfound.py` | 7 | **Missing `.feature` file** |
| `test_rrd_dynamic_profile_rdm_node_length_exceeded.py` | 6 | **Missing `.feature` file** |
| `test_rrd_dynamic_subcategory_report.py` | 7 | **Missing `.feature` file** |
| `test_rrd_profile_data.py` | 3 | **Missing `.feature` file** |

### 2.4 Orphan Features (feature exists, no test file)

| Feature File | Scenarios | Note |
|---|:---:|---|
| `rrd_append_dynamic_profile_static_not_found.feature` | 4 | **Missing test file** |
| `rrd_dynamic_profile_node_length_exceeded.feature` | 4 | **Missing test file** |
| `rrd_dynamic_profile_subcategory_report.feature` | 5 | **Missing test file** |

### 2.5 RRD Testcase Coverage

> Status is an evidence review, not inferred from filename matching. A linked test is not marked covered unless its assertions verify the requested behavior.

| # | RRD testcase | Coverage status | Feature files | Test files | Coverage notes |
|:---:|---|:---:|---|---|---|
| 1 | Static Profile Debug Report | COVERED | `rrd_static_profile_report.feature` | `test_rrd_static_profile_report.py` | Static IssueType parsing, command execution, output, and upload invocation are asserted. |
| 2 | Enable the RRD via TR-181 | COVERED | `rrd_start_control.feature` | `test_rrd_start_control.py` | Enable parameter is queried and daemon startup is verified. |
| 3 | Disable the RRD via TR-181 | COVERED | `rrd_start_control.feature` | `test_rrd_start_control.py` | Disabled parameter behavior and daemon stop are verified. |
| 4 | Enable RRD via RFC and persist after reboot | GAP | — | — | No reboot-persistence scenario was found. |
| 5 | Validate RRD Debug Report update using RBUS and TR-181 commands | COVERED | `rrd_static_profile_report.feature` | `test_rrd_static_profile_report.py` | The test sets IssueType with rbuscli and checks event delivery and report output. |
| 6 | Report upload after running the requested Issue command | COVERED | `rrd_debug_report_upload.feature` | `test_rrd_debug_report_upload.py` | The test checks tarball creation and download from the mockxconf upload endpoint. |
| 7 | Dynamic Profile Debug Report | COVERED | `rrd_dynamic_profile_report.feature` | `test_rrd_dynamic_profile_report.py` | Dynamic profile lookup, command execution, and report upload flow are exercised. |
| 8 | Append static commands from the dynamic profile for the same IssueType | COVERED | `rrd_append_report.feature` | `test_rrd_append_report.py` | The test checks the merged static and dynamic command path. |
| 9 | Deep-sleep Profile Debug Report including WebConfig/IARM events | PARTIAL | `rrd_deepsleep_static_report.feature` | `test_rrd_deepsleep_static_report.py` | The DEEPSLEEP profile report is tested; the WebConfig/IARM event path is not directly verified. |
| 10 | Set/Get category-level query from the static JSON profile | PARTIAL | — | `test_rrd_profile_data.py` | The test sets a category and checks command success, but does not assert the returned category contents. |
| 11 | Set/Get all data from the static JSON profile | PARTIAL | — | `test_rrd_profile_data.py` | The test requests all data but validates only the response type when output is non-empty. |
| 12 | Support Issue Debug Report requests with UUID | GAP | — | — | No UUID-bearing request scenario or assertion was found. |
| 13 | Support Issue Debug Report requests without UUID | GAP | — | — | No explicit UUID-absent request scenario was found. |
| 14 | Use the request UUID for report upload after command execution | GAP | — | — | No assertion connects a request UUID to the upload operation. |
| 15 | Confirm report upload to the S3 portal | PARTIAL | `rrd_debug_report_upload.feature` | `test_rrd_debug_report_upload.py` | Upload and download are verified against mockxconf, not the S3 portal. |
| 16 | Match the request UUID with the uploaded report tarball | GAP | — | — | The tarball is checked for existence/download, but its UUID is not compared with the request. |
| 17 | Execute a valid static IssueType debug command | COVERED | `rrd_static_profile_report.feature` | `test_rrd_static_profile_report.py` | The static profile test checks command validation, output creation, and service execution. |
| 18 | Verify all static IssueTypes | PARTIAL | `rrd_static_profile_report.feature`, `rrd_static_profile_category_report.feature` | `test_rrd_static_profile_report.py`, `test_rrd_static_profile_category_report.py` | Representative static IssueTypes are exercised; exhaustive profile coverage is not shown. |
| 19 | Attempt to execute a forbidden command | COVERED | `rrd_harmful_command_static_report.feature` | `test_rrd_harmful_command_static_report.py` | The harmful-command test checks that command execution is aborted and upload skipped. |
| 20 | Send a malformed or empty command | PARTIAL | `rrd_empty_issuetype_event.feature` | `test_rrd_empty_issuetype_event.py` | An empty IssueType is tested; malformed command input is not explicitly covered. |
| 21 | Download a dynamic debug profile via RDM | PARTIAL | `rrd_dynamic_profile_missing_report.feature` | `test_rrd_dynamic_profile_missing_report.py` | The missing-profile path requests an RDM download; successful package delivery and installation are not verified. |
| 22 | Trigger a non-existent or misspelled IssueType | PARTIAL | `rrd_dynamic_profile_missing_report.feature` | `test_rrd_dynamic_profile_missing_report.py` | Missing static/dynamic IssueType behavior is exercised, but misspellings are not isolated as a case. |
| 23 | Validate log/tarball creation and upload | COVERED | `rrd_debug_report_upload.feature` | `test_rrd_debug_report_upload.py` | The feature and test check report archive creation, upload to mockxconf, and download. |
| 24 | Simulate S3 failure and check retry/logging | GAP | — | — | No S3 failure/retry scenario was found. |
| 25 | Try running sensitive system commands | PARTIAL | `rrd_harmful_command_static_report.feature` | `test_rrd_harmful_command_static_report.py` | Harmful-command rejection is tested, but the requested sensitive-command set is not enumerated. |
| 26 | Send a debug command while in deep sleep | PARTIAL | `rrd_deepsleep_static_report.feature` | `test_rrd_deepsleep_static_report.py` | DEEPSLEEP report processing is tested, but command timing during actual sleep is not asserted. |
| 27 | Validate resume on wake | GAP | — | — | No resume-on-wake assertion was found. |
| 28 | Flood remote debugger with commands (stress test) | GAP | — | — | No command-flood or stress test was found. |
| 29 | Delete/corrupt configuration and simulate network loss | PARTIAL | `rrd_corrupted_static_profile_report.feature` | `test_rrd_corrupted_static_profile_report.py` | Corrupted static JSON is tested; network-loss behavior is not. |
| 30 | Run debugger alongside other system daemons | GAP | — | — | No multi-daemon coexistence test was found. |
| 31 | Firmware upgrade and disk stress | GAP | — | — | No firmware-upgrade or disk-stress scenario was found. |
| 32 | Set/Get with a misspelled category | PARTIAL | — | `test_rrd_profile_data.py` | A non-existent category is tried, but the test does not assert the resulting profile contents. |
| 33 | Set/Get with an empty category | PARTIAL | — | `test_rrd_profile_data.py` | An empty category is tried, but the test does not assert the resulting profile contents. |

---

## 3. Source Module Coverage (lcov)

| Module | Lines | Functions | Coverage Bar |
|---|:---:|:---:|---|
| `rrdMain.c` | 60.3% (38/63) | 75.0% (3/4) | `████████████░░░░░░░░` |
| `rrdInterface.c` | 44.0% (189/430) | 63.6% (14/22) | `█████████░░░░░░░░░░░` |
| `rrdEventProcess.c` | 68.8% (240/349) | 81.8% (9/11) | `██████████████░░░░░░` |
| `rrdJsonParser.c` | 81.8% (405/495) | 86.7% (13/15) | `████████████████░░░░` |
| `rrdRunCmdThread.c` | 55.2% (117/212) | 60.0% (6/10) | `███████████░░░░░░░░░` |
| `rrdCommandSanity.c` | 93.1% (67/72) | 100.0% (3/3) | `███████████████████░` |
| `rrdDynamic.c` | 40.4% (59/146) | 40.0% (2/5) | `████████░░░░░░░░░░░░` |
| `rrdExecuteScript.c` | 94.1% (16/17) | 100.0% (2/2) | `███████████████████░` |
| `rrdMsgPackDecoder.c` | 0.0% (0/280) | 0.0% (0/15) | `░░░░░░░░░░░░░░░░░░░░` |
| `rrd_config.c` | 42.8% (95/222) | 66.7% (6/9) | `█████████░░░░░░░░░░░` |
| `rrd_sysinfo.c` | 32.1% (36/112) | 33.3% (2/6) | `██████░░░░░░░░░░░░░░` |
| `rrd_logproc.c` | 48.6% (36/74) | 75.0% (3/4) | `██████████░░░░░░░░░░` |
| `rrd_archive.c` | 47.2% (126/267) | 72.7% (8/11) | `█████████░░░░░░░░░░░` |
| `rrd_upload.c` | 40.7% (37/91) | 60.0% (3/5) | `████████░░░░░░░░░░░░` |
| `rrdIarmEvents.c` | 8.6% (12/140) | 20.0% (1/5) | `██░░░░░░░░░░░░░░░░░░` |
| `uploadRRDLogs.c` | 54.7% (35/64) | 100.0% (1/1) | `███████████░░░░░░░░░` |

---

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


---

## 5. Scenario-to-Test Gap Analysis

| Feature File | Scenarios | Tests | Missing |
|---|:---:|:---:|:---:|
| `rrd_c_api_upload.feature` | 21 | 5 | **16** |
| `rrd_start_control.feature` | 2 | 1 | **1** |

**Orphan tests** (no feature file — behavior is tested but not documented):

- `test_rrd_append_dynamic_profile_static_notfound.py`
- `test_rrd_dynamic_profile_rdm_node_length_exceeded.py`
- `test_rrd_dynamic_subcategory_report.py`
- `test_rrd_profile_data.py`

---

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


---

## 7. Test Infrastructure Notes

| Interface | Tool | Used By |
|---|---|---|
| RBUS event trigger | `rbuscli set` | All event-based tests |
| RBUS profile data | `rbuscli set/get` | `test_rrd_profile_data.py` |
| Log scraping | `grep_rrdlogs()` in `helper_functions.py` | All tests |
| Process control | `pidof`, `kill`, `nohup` | Setup / teardown |
| Mock upload server | Mock xconf server | `test_rrd_debug_report_upload.py` |
| Mock upload script | `uploadSTBLogs.sh` | Upload tests |

- **Runner:** `pytest`, sequential per file
- **Shared state:** tests within a file depend on prior test side effects
- **Coverage run:** `kill_rrd()` sends `SIGTERM` when `RRD_COVERAGE_MODE` is set so gcov flushes

---

## 8. Appendix: File Inventory

### Feature Files

| # | File | Scenarios |
|:---:|---|:---:|
| 1 | `rrd_append_dynamic_profile_static_not_found.feature` | 4 |
| 2 | `rrd_append_report.feature` | 4 |
| 3 | `rrd_background_cmd_static_profile_report.feature` | 5 |
| 4 | `rrd_c_api_upload.feature` | 21 |
| 5 | `rrd_corrupted_static_profile_report.feature` | 4 |
| 6 | `rrd_debug_report_upload.feature` | 6 |
| 7 | `rrd_deepsleep_static_report.feature` | 2 |
| 8 | `rrd_dynamic_profile_missing_report.feature` | 4 |
| 9 | `rrd_dynamic_profile_node_length_exceeded.feature` | 4 |
| 10 | `rrd_dynamic_profile_report.feature` | 5 |
| 11 | `rrd_dynamic_profile_subcategory_report.feature` | 5 |
| 12 | `rrd_empty_issuetype_event.feature` | 2 |
| 13 | `rrd_harmful_command_static_report.feature` | 5 |
| 14 | `rrd_single_instance.feature` | 1 |
| 15 | `rrd_start_control.feature` | 2 |
| 16 | `rrd_start_subscribe_and_wait.feature` | 1 |
| 17 | `rrd_static_profile_category_report.feature` | 5 |
| 18 | `rrd_static_profile_missing_command_report.feature` | 5 |
| 19 | `rrd_static_profile_report.feature` | 5 |
| 20 | `test_rrd_dynamic_profile_harmful_report.feature` | 5 |
| 21 | `test_rrd_static_profile_report_with_suffix.feature` | 4 |
| 22 | `test_rrd_static_profile_report_with_suffix_negative_case.feature` | 4 |
| | **Total** | **103** |

### Test Files

| # | File | Tests |
|:---:|---|:---:|
| 1 | `test_rrd_append_dynamic_profile_static_notfound.py` | 7 |
| 2 | `test_rrd_append_report.py` | 7 |
| 3 | `test_rrd_background_cmd_static_profile_report.py` | 5 |
| 4 | `test_rrd_c_api_upload.py` | 5 |
| 5 | `test_rrd_corrupted_static_profile_report.py` | 4 |
| 6 | `test_rrd_debug_report_upload.py` | 6 |
| 7 | `test_rrd_deepsleep_static_report.py` | 5 |
| 8 | `test_rrd_dynamic_profile_harmful_report.py` | 7 |
| 9 | `test_rrd_dynamic_profile_missing_report.py` | 7 |
| 10 | `test_rrd_dynamic_profile_rdm_node_length_exceeded.py` | 6 |
| 11 | `test_rrd_dynamic_profile_report.py` | 9 |
| 12 | `test_rrd_dynamic_subcategory_report.py` | 7 |
| 13 | `test_rrd_empty_issuetype_event.py` | 2 |
| 14 | `test_rrd_harmful_command_static_report.py` | 5 |
| 15 | `test_rrd_profile_data.py` | 3 |
| 16 | `test_rrd_single_instance.py` | 3 |
| 17 | `test_rrd_start_control.py` | 1 |
| 18 | `test_rrd_start_subscribe_and_wait.py` | 4 |
| 19 | `test_rrd_static_profile_category_report.py` | 5 |
| 20 | `test_rrd_static_profile_missing_command_report.py` | 5 |
| 21 | `test_rrd_static_profile_report.py` | 5 |
| 22 | `test_rrd_static_profile_report_with_suffix.py` | 5 |
| 23 | `test_rrd_static_profile_report_with_suffix_negative_case.py` | 5 |
| | **Total** | **118** |
