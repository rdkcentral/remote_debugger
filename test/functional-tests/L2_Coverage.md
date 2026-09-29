# Remote Debugger L2 Coverage Report

**Generated:** 2026-08-14  
**Component:** `remotedebugger` (`src/`)  
**Test suite:** `test/functional-tests/`  
**Coverage tool:** lcov (source-level instrumentation via `--coverage`)

---

## 1. Executive Summary

| Metric | Value |
|---|:---:|
| Feature files | 22 |
| Feature scenarios | 103 |
| Test files (pytest) | 23 |
| Test functions (`test_*`) | 118 |
| Feature → Test mapped pairs | 19 / 22 (+4 orphan tests) |
| **Line coverage (lcov)** | **49.4%** (1487 of 3012 lines) |
| **Branch coverage (lcov)** | **36.8%** (503 of 1365 branches) |
| **Function coverage (lcov)** | **60.3%** (76 of 126 functions) |

---

## 2. Feature ↔ Test Mapping

### 2.1 Mapped Pairs

| # | Feature File | Scenarios | Test File | Tests | Gap |
|:---:|---|:---:|---|:---:|:---:|
| 1 | `rrd_append_report.feature` | 4 | `test_rrd_append_report.py` | 7 | — |
| 2 | `rrd_background_cmd_static_profile_report.feature` | 5 | `test_rrd_background_cmd_static_profile_report.py` | 5 | — |
| 3 | `rrd_c_api_upload.feature` | 21 | `test_rrd_c_api_upload.py` | 5 | **16 missing** |
| 4 | `rrd_corrupted_static_profile_report.feature` | 4 | `test_rrd_corrupted_static_profile_report.py` | 4 | — |
| 5 | `rrd_debug_report_upload.feature` | 6 | `test_rrd_debug_report_upload.py` | 6 | — |
| 6 | `rrd_deepsleep_static_report.feature` | 2 | `test_rrd_deepsleep_static_report.py` | 5 | — |
| 7 | `rrd_dynamic_profile_missing_report.feature` | 4 | `test_rrd_dynamic_profile_missing_report.py` | 7 | — |
| 8 | `rrd_dynamic_profile_report.feature` | 5 | `test_rrd_dynamic_profile_report.py` | 9 | — |
| 9 | `rrd_empty_issuetype_event.feature` | 2 | `test_rrd_empty_issuetype_event.py` | 2 | — |
| 10 | `rrd_harmful_command_static_report.feature` | 5 | `test_rrd_harmful_command_static_report.py` | 5 | — |
| 11 | `rrd_single_instance.feature` | 1 | `test_rrd_single_instance.py` | 3 | — |
| 12 | `rrd_start_control.feature` | 2 | `test_rrd_start_control.py` | 1 | **1 missing** |
| 13 | `rrd_start_subscribe_and_wait.feature` | 1 | `test_rrd_start_subscribe_and_wait.py` | 4 | — |
| 14 | `rrd_static_profile_category_report.feature` | 5 | `test_rrd_static_profile_category_report.py` | 5 | — |
| 15 | `rrd_static_profile_missing_command_report.feature` | 5 | `test_rrd_static_profile_missing_command_report.py` | 5 | — |
| 16 | `rrd_static_profile_report.feature` | 5 | `test_rrd_static_profile_report.py` | 5 | — |
| 17 | `test_rrd_dynamic_profile_harmful_report.feature` | 5 | `test_rrd_dynamic_profile_harmful_report.py` | 7 | — |
| 18 | `test_rrd_static_profile_report_with_suffix.feature` | 4 | `test_rrd_static_profile_report_with_suffix.py` | 5 | — |
| 19 | `test_rrd_static_profile_report_with_suffix_negative_case.feature` | 4 | `test_rrd_static_profile_report_with_suffix_negative_case.py` | 5 | — |
| | **Totals** | **90** | | **95** | |

### 2.2 Orphan Tests (test exists, no feature file)

| Test File | Tests | Note |
|---|:---:|---|
| `test_rrd_append_dynamic_profile_static_notfound.py` | 7 | **Missing `.feature` file** |
| `test_rrd_dynamic_profile_rdm_node_length_exceeded.py` | 6 | **Missing `.feature` file** |
| `test_rrd_dynamic_subcategory_report.py` | 7 | **Missing `.feature` file** |
| `test_rrd_profile_data.py` | 3 | **Missing `.feature` file** |

### 2.3 Orphan Features (feature exists, no test file)

| Feature File | Scenarios | Note |
|---|:---:|---|
| `rrd_append_dynamic_profile_static_not_found.feature` | 4 | **Missing test file** |
| `rrd_dynamic_profile_node_length_exceeded.feature` | 4 | **Missing test file** |
| `rrd_dynamic_profile_subcategory_report.feature` | 5 | **Missing test file** |

---

## 3. Source Module Coverage (lcov)

| Module | Lines | Functions | Branches | Coverage Bar |
|---|:---:|:---:|:---:|---|
| `rrdMain.c` | 60.3% (38/63) | 100.0% (4/4) | 42.9% (6/14) | `████████████░░░░░░░░` |
| `rrdInterface.c` | 42.4% (181/427) | 59.1% (13/22) | 33.3% (56/168) | `████████░░░░░░░░░░░░` |
| `rrdEventProcess.c` | 69.3% (232/335) | 81.8% (9/11) | 48.6% (68/140) | `██████████████░░░░░░` |
| `rrdJsonParser.c` | 81.6% (400/490) | 86.7% (13/15) | 66.0% (128/194) | `████████████████░░░░` |
| `rrdRunCmdThread.c` | 55.2% (117/212) | 60.0% (6/10) | 34.5% (20/58) | `███████████░░░░░░░░░` |
| `rrdCommandSanity.c` | 93.1% (67/72) | 100.0% (3/3) | 71.9% (23/32) | `███████████████████░` |
| `rrdDynamic.c` | 40.4% (59/146) | 40.0% (2/5) | 24.6% (14/57) | `████████░░░░░░░░░░░░` |
| `rrdExecuteScript.c` | 94.1% (16/17) | 100.0% (2/2) | 75.0% (6/8) | `███████████████████░` |
| `rrdMsgPackDecoder.c` | 0.0% (0/280) | 0.0% (0/15) | 0.0% (0/138) | `░░░░░░░░░░░░░░░░░░░░` |
| `rrd_config.c` | 42.8% (95/222) | 66.7% (6/9) | 27.6% (53/192) | `█████████░░░░░░░░░░░` |
| `rrd_sysinfo.c` | 32.1% (36/112) | 33.3% (2/6) | 19.6% (11/56) | `██████░░░░░░░░░░░░░░` |
| `rrd_logproc.c` | 48.6% (36/74) | 75.0% (3/4) | 47.9% (23/48) | `██████████░░░░░░░░░░` |
| `rrd_archive.c` | 47.2% (126/267) | 80.0% (8/10) | 46.5% (66/142) | `█████████░░░░░░░░░░░` |
| `rrd_upload.c` | 40.7% (37/91) | 60.0% (3/5) | 30.4% (14/46) | `████████░░░░░░░░░░░░` |
| `rrdIarmEvents.c` | 8.6% (12/140) | 25.0% (1/4) | 8.3% (4/48) | `██░░░░░░░░░░░░░░░░░░` |
| `uploadRRDLogs.c` | 54.7% (35/64) | 100.0% (1/1) | 45.8% (11/24) | `███████████░░░░░░░░░` |

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

## 7. Appendix: File Inventory

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
