# Remote Debugger L2 Test Coverage Report

**Generated:** 2026-05-19
**Component:** `remotedebugger` (src/)
**Test Suite:** `test/functional-tests/`

---

**Test Coverage Summary**

```Total source functions (approx): ~123
Functions with direct L2 coverage: ~55
Functions with indirect L2 coverage: ~32
Functions with no L2 coverage: ~36

Active L2 test functions: 118
Disabled L2 test functions: 0
Active feature scenarios: 103
Proposed new test scenarios: 22

High priority: 11
Medium priority: 7
Low priority: 4

Test files active: 23
Test files disabled (commented out): 0

Estimated current L2 functional coverage: ~55%
Target L2 functional coverage: ~80%
```
---

## 1. Feature File ↔ Test File Mapping

### Fully Mapped (feature + test exist)

| # | Feature File | Scenarios | Test File | Test Functions | Status |
|:---:|---|:---:|---|:---:|:---:|
| 1 | `rrd_start_subscribe_and_wait.feature` | 1 | `test_rrd_start_subscribe_and_wait.py` | 4 | PASS |
| 2 | `rrd_start_control.feature` | 2 | `test_rrd_start_control.py` | 1 | PASS |
| 3 | `rrd_single_instance.feature` | 1 | `test_rrd_single_instance.py` | 3 | PASS |
| 4 | `rrd_static_profile_report.feature` | 5 | `test_rrd_static_profile_report.py` | 5 | PASS |
| 5 | `rrd_static_profile_category_report.feature` | 5 | `test_rrd_static_profile_category_report.py` | 5 | PASS |
| 6 | `test_rrd_static_profile_report_with_suffix.feature` | 4 | `test_rrd_static_profile_report_with_suffix.py` | 5 | PASS |
| 7 | `test_rrd_static_profile_report_with_suffix_negative_case.feature` | 4 | `test_rrd_static_profile_report_with_suffix_negative_case.py` | 5 | PASS |
| 8 | `rrd_background_cmd_static_profile_report.feature` | 5 | `test_rrd_background_cmd_static_profile_report.py` | 5 | PASS |
| 9 | `rrd_dynamic_profile_report.feature` | 5 | `test_rrd_dynamic_profile_report.py` | 9 | PASS |
| 10 | `rrd_dynamic_profile_subcategory_report.feature` | 5 | `test_rrd_dynamic_subcategory_report.py` | 7 | PASS |
| 11 | `rrd_dynamic_profile_missing_report.feature` | 4 | `test_rrd_dynamic_profile_missing_report.py` | 7 | PASS |
| 12 | `rrd_append_report.feature` | 4 | `test_rrd_append_report.py` | 7 | PASS |
| 13 | `rrd_append_dynamic_profile_static_not_found.feature` | 4 | `test_rrd_append_dynamic_profile_static_notfound.py` | 7 | PASS |
| 14 | `rrd_harmful_command_static_report.feature` | 5 | `test_rrd_harmful_command_static_report.py` | 5 | PASS |
| 15 | `test_rrd_dynamic_profile_harmful_report.feature` | 5 | `test_rrd_dynamic_profile_harmful_report.py` | 7 | PASS |
| 16 | `rrd_corrupted_static_profile_report.feature` | 4 | `test_rrd_corrupted_static_profile_report.py` | 4 | PASS |
| 17 | `rrd_static_profile_missing_command_report.feature` | 5 | `test_rrd_static_profile_missing_command_report.py` | 5 | PASS |
| 18 | `rrd_empty_issuetype_event.feature` | 2 | `test_rrd_empty_issuetype_event.py` | 2 | PASS |
| 19 | `rrd_deepsleep_static_report.feature` | 2 | `test_rrd_deepsleep_static_report.py` | 5 | PASS |
| 20 | `rrd_debug_report_upload.feature` | 6 | `test_rrd_debug_report_upload.py` | 5 | PASS |
| 21 | `rrd_c_api_upload.feature` | 21 | `test_rrd_c_api_upload.py` | 5 | GAP |
| | **Totals** | **97** | | **112** | |

> **Note:** Scenario count for `rrd_c_api_upload.feature` (21) far exceeds its test function count (5). See Section 3 for details.

### Orphan Tests (test exists, no matching feature file)

| Test File | Test Functions | Description | Gap |
|---|:---:|---|---|
| `test_rrd_profile_data.py` | 3 | RBUS profile data SET/GET via `rbuscli` | **Missing `.feature` file** |

### Orphan Features (feature exists, no matching test file)

None — all 21 feature files have corresponding test files.

---

## 2. Per-Behavior Coverage Detail

### 2.1 Daemon Lifecycle

| Behavior | Feature | Test | Covered |
|---|---|---|:---:|
| RBUS subscription + event wait | `rrd_start_subscribe_and_wait` | `test_rrd_start_subscribe_and_wait` | YES |
| RFC enable → daemon starts | `rrd_start_control` | `test_rrd_start_control` | YES |
| RFC disable → daemon stops | `rrd_start_control` | `test_rrd_start_control` | YES |
| Single instance enforcement | `rrd_single_instance` | `test_rrd_single_instance` | YES |
| Message queue creation failure | — | — | **NO** |
| Event thread creation failure | — | — | **NO** |
| Signal handling / graceful shutdown | — | — | **NO** |
| Device info file read failure | — | — | **NO** |

### 2.2 Static Profile Processing

| Behavior | Feature | Test | Covered |
|---|---|---|:---:|
| Config file exists check | `rrd_static_profile_report` | `test_rrd_static_profile_report` | YES |
| Output directory exists check | `rrd_static_profile_report` | `test_rrd_static_profile_report` | YES |
| IssueType event trigger + message flow | `rrd_static_profile_report` | `test_rrd_static_profile_report` | YES |
| JSON parse success + command execution | `rrd_static_profile_report` | `test_rrd_static_profile_report` | YES |
| Upload report success/failure | `rrd_static_profile_report` | `test_rrd_static_profile_report` | YES |
| Category-only issue type (all sub-nodes) | `rrd_static_profile_category_report` | `test_rrd_static_profile_category_report` | YES |
| Suffixed issue type | `test_rrd_static_profile_report_with_suffix` | `test_rrd_static_profile_report_with_suffix` | YES |
| Overlength suffix (negative) | `test_rrd_static_profile_report_with_suffix_negative_case` | `test_rrd_static_profile_report_with_suffix_negative_case` | YES |
| Background command execution | `rrd_background_cmd_static_profile_report` | `test_rrd_background_cmd_static_profile_report` | YES |
| Missing command in profile | `rrd_static_profile_missing_command_report` | `test_rrd_static_profile_missing_command_report` | YES |
| Corrupted/invalid JSON profile | `rrd_corrupted_static_profile_report` | `test_rrd_corrupted_static_profile_report` | YES |

### 2.3 Dynamic Profile Processing

| Behavior | Feature | Test | Covered |
|---|---|---|:---:|
| Dynamic profile fallback (static miss) | `rrd_dynamic_profile_report` | `test_rrd_dynamic_profile_report` | YES |
| Dynamic subcategory | `rrd_dynamic_profile_subcategory_report` | `test_rrd_dynamic_subcategory_report` | YES |
| Dynamic profile missing → RDM trigger | `rrd_dynamic_profile_missing_report` | `test_rrd_dynamic_profile_missing_report` | YES |
| Append mode (static + dynamic) | `rrd_append_report` | `test_rrd_append_report` | YES |
| Append when static not found | `rrd_append_dynamic_profile_static_not_found` | `test_rrd_append_dynamic_profile_static_notfound` | YES |
| RDM download event (cache miss) | — | — | **NO** |
| Dynamic profile JSON parse failure | — | — | **NO** |

### 2.4 Harmful Command Detection

| Behavior | Feature | Test | Covered |
|---|---|---|:---:|
| Static profile harmful command abort | `rrd_harmful_command_static_report` | `test_rrd_harmful_command_static_report` | YES |
| Dynamic profile harmful command abort | `test_rrd_dynamic_profile_harmful_report` | `test_rrd_dynamic_profile_harmful_report` | YES |
| Macro replacement edge cases | — | — | **NO** |
| Background command modification | — | — | **PARTIAL** (via background cmd test) |

### 2.5 Event Handling

| Behavior | Feature | Test | Covered |
|---|---|---|:---:|
| IssueType RBUS event | Multiple features | Multiple tests | YES |
| Empty IssueType event | `rrd_empty_issuetype_event` | `test_rrd_empty_issuetype_event` | YES |
| Deep sleep event | `rrd_deepsleep_static_report` | `test_rrd_deepsleep_static_report` | YES |
| WebCfg event (MsgPack decode) | — | — | **NO** |
| WebCfg corrupted data | — | — | **NO** |
| Multiple simultaneous IssueType events | — | — | **NO** |
| Invalid deep sleep event type | — | — | **NO** |

### 2.6 Upload & Archive

| Behavior | Feature | Test | Covered |
|---|---|---|:---:|
| Upload via shell script | `rrd_debug_report_upload` | `test_rrd_debug_report_upload` | YES |
| Upload + download validation | `rrd_debug_report_upload` | `test_rrd_debug_report_upload` | YES |
| C API `rrd_upload_orchestrate` (happy path) | `rrd_c_api_upload` | `test_rrd_c_api_upload` | YES |
| C API NULL parameters | `rrd_c_api_upload` | — | **GAP** |
| C API empty directory | `rrd_c_api_upload` | — | **GAP** |
| C API non-existent directory | `rrd_c_api_upload` | — | **GAP** |
| C API config loading validation | `rrd_c_api_upload` | — | **GAP** |
| C API MAC retrieval | `rrd_c_api_upload` | — | **GAP** |
| C API timestamp generation | `rrd_c_api_upload` | — | **GAP** |
| C API issue type sanitization | `rrd_c_api_upload` | — | **GAP** |
| C API archive creation | `rrd_c_api_upload` | — | **GAP** |
| C API upload execution | `rrd_c_api_upload` | — | **GAP** |
| C API cleanup after success/failure | `rrd_c_api_upload` | — | **GAP** |
| C API concurrent upload lock | `rrd_c_api_upload` | — | **GAP** |
| C API LOGUPLOAD_ENABLE | `rrd_c_api_upload` | — | **GAP** |
| C API end-to-end with RFC trigger | `rrd_c_api_upload` | — | **GAP** |
| C API error propagation | `rrd_c_api_upload` | — | **GAP** |
| Upload lock file contention | — | — | **NO** |
| Archive CPU throttle logic | — | — | **NO** |

### 2.7 Profile Data SET/GET (RBUS)

| Behavior | Feature | Test | Covered |
|---|---|---|:---:|
| `setProfileData` / `getProfileData` | — | `test_rrd_profile_data` | **PARTIAL** (no feature) |
| Profile category load error | — | — | **NO** |
| Profile file write error | — | — | **NO** |

---

## 3. Feature ↔ Test Gap Analysis

### 3.1 `rrd_c_api_upload.feature` — Major Scenario-to-Test Gap

The feature file documents **21 scenarios** covering the full `rrd_upload_orchestrate` C API, but the test file `test_rrd_c_api_upload.py` only implements **5 test functions** that cover the basic end-to-end flow (config check, dir check, start, trigger event, upload report).

**Missing test implementations for feature scenarios:**

| Feature Scenario | Test Status |
|---|---|
| Validate rrd_upload_orchestrate C API with valid parameters | Covered (within `test_remote_debugger_trigger_event`) |
| Test rrd_upload_orchestrate with NULL upload directory | **NOT IMPLEMENTED** |
| Test rrd_upload_orchestrate with NULL issue type | **NOT IMPLEMENTED** |
| Test rrd_upload_orchestrate with empty upload directory | **NOT IMPLEMENTED** |
| Test rrd_upload_orchestrate with non-existent directory | **NOT IMPLEMENTED** |
| Test rrd_upload_orchestrate configuration loading | **NOT IMPLEMENTED** |
| Test rrd_upload_orchestrate MAC address retrieval | **NOT IMPLEMENTED** |
| Test rrd_upload_orchestrate timestamp generation | **NOT IMPLEMENTED** |
| Test rrd_upload_orchestrate issue type sanitization | **NOT IMPLEMENTED** |
| Test rrd_upload_orchestrate archive creation | **NOT IMPLEMENTED** |
| Test rrd_upload_orchestrate upload execution | **NOT IMPLEMENTED** |
| Test rrd_upload_orchestrate cleanup after success | **NOT IMPLEMENTED** |
| Test rrd_upload_orchestrate cleanup after upload failure | **NOT IMPLEMENTED** |
| Test uploadDebugoutput wrapper function | **NOT IMPLEMENTED** |
| Test concurrent upload lock handling | **NOT IMPLEMENTED** |
| Test rrd_upload_orchestrate with LOGUPLOAD_ENABLE issue type | **NOT IMPLEMENTED** |
| Test remote debugger end-to-end with RFC trigger | Covered |
| Test upload report validation with success path | Covered (within `test_remotedebugger_upload_report`) |
| Test upload report validation with failure path | Covered (within `test_remotedebugger_upload_report`) |
| Test upload report with legacy log compatibility | **NOT IMPLEMENTED** |
| Test rrd_upload_orchestrate error propagation | **NOT IMPLEMENTED** |

**Gap: 16 of 21 scenarios have no dedicated test implementation.**

### 3.2 `test_rrd_profile_data.py` — Orphan Test (No Feature File)

This test file contains 3 test functions exercising `rbuscli set/get` on profile data RBUS elements (`setProfileData` / `getProfileData`). No corresponding `.feature` file exists.

**Recommendation:** Create `rrd_profile_data.feature` to document this behavior.

### 3.3 Scenario Count vs Test Count Discrepancy

Some test files implement more test functions than their feature has scenarios, due to setup/teardown and prerequisite tests being split more granularly:

| Feature File | Scenarios | Test File | Tests | Delta |
|---|:---:|---|:---:|:---:|
| `rrd_start_subscribe_and_wait` | 1 | `test_rrd_start_subscribe_and_wait` | 4 | +3 |
| `rrd_single_instance` | 1 | `test_rrd_single_instance` | 3 | +2 |
| `rrd_dynamic_profile_report` | 5 | `test_rrd_dynamic_profile_report` | 9 | +4 |
| `rrd_dynamic_profile_missing_report` | 4 | `test_rrd_dynamic_profile_missing_report` | 7 | +3 |
| `rrd_append_report` | 4 | `test_rrd_append_report` | 7 | +3 |
| `rrd_append_dynamic_profile_static_not_found` | 4 | `test_rrd_append_dynamic_profile_static_notfound` | 7 | +3 |
| `rrd_dynamic_profile_harmful_report` | 5 | `test_rrd_dynamic_profile_harmful_report` | 7 | +2 |
| `rrd_deepsleep_static_report` | 2 | `test_rrd_deepsleep_static_report` | 5 | +3 |
| `rrd_c_api_upload` | 21 | `test_rrd_c_api_upload` | 5 | **-16** |

The **only deficit** is `rrd_c_api_upload` where the feature documents far more scenarios than tests implement.

---

## 4. Source Module Coverage Analysis

### 4.1 Module-Level Coverage Summary

| Source Module | Happy Path | Error Paths | L2 Coverage |
|---|:---:|:---:|---|
| `rrdMain.c` | YES | NO | Startup, RFC check, event thread — tested. Msgqueue/thread failures — not tested. |
| `rrdInterface.c` | YES | NO | RBUS registration, event handlers — tested. Registration failures, file I/O errors — not tested. |
| `rrdEventProcess.c` | YES | PARTIAL | Static/dynamic/append/deepsleep — tested. WebCfg event, malloc failures — not tested. |
| `rrdJsonParser.c` | YES | PARTIAL | Valid/corrupted/missing JSON — tested. Dir creation failures, malloc failures — not tested. |
| `rrdRunCmdThread.c` | YES | NO | Command execution, output files — tested. File write errors, systemd-run failures — not tested. |
| `rrdCommandSanity.c` | YES | NO | Harmful command detection — tested. Macro replacement errors — not tested. |
| `rrdDynamic.c` | YES | NO | Dynamic profile, deep sleep — tested. RBUS set failure, invalid event type — not tested. |
| `rrdExecuteScript.c` | YES | NO | Upload orchestration — tested. Script exec failure, API failure — not tested. |
| `rrdMsgPackDecoder.c` | NO | NO | **Entirely untested** — WebCfg MsgPack decode not exercised by any L2 test. |
| `rrd_config.c` | INDIRECT | NO | Indirectly tested via daemon startup. Config parse errors, RFC query failures — not tested. |
| `rrd_sysinfo.c` | INDIRECT | NO | Indirectly tested via upload flow. MAC/timestamp retrieval errors — not tested. |
| `rrd_logproc.c` | INDIRECT | NO | Indirectly tested via upload flow. Log dir validation errors — not tested. |
| `rrd_archive.c` | INDIRECT | NO | Indirectly tested via upload flow. CPU throttle, archive errors — not tested. |
| `rrd_upload.c` | INDIRECT | NO | Indirectly tested via upload flow. Lock errors, cleanup failures — not tested. |
| `rrdIarmEvents.c` | PARTIAL | NO | Deep sleep event — tested. Other IARM events — not tested. |
| `uploadRRDLogs.c` | INDIRECT | NO | Entry point tested via daemon trigger. Direct API error paths — not tested. |

### 4.2 Completely Untested Source Behaviors

| Priority | Behavior | Source Module | Required Infrastructure |
|:---:|---|---|---|
| **P1** | WebCfg event (MsgPack decode + dispatch) | `rrdMsgPackDecoder.c`, `rrdEventProcess.c` | WebConfig mock, base64-encoded MsgPack payload |
| **P1** | C API upload error paths (NULL params, empty dir, non-existent dir) | `rrd_upload.c` | Direct C API invocation or test binary |
| **P1** | Profile data SET/GET feature documentation | `rrdInterface.c` | Feature file creation only |
| **P2** | Concurrent upload lock contention | `rrd_upload.c` | Parallel upload trigger + lock file manipulation |
| **P2** | Archive CPU usage throttle | `rrd_archive.c` | CPU load simulation |
| **P2** | Configuration fallback chain (RFC → DCM → dcm.properties) | `rrd_config.c` | Config file manipulation |
| **P2** | RDM download event with cache miss | `rrdInterface.c`, `rrdDynamic.c` | RDM mock, empty cache |
| **P3** | RBUS registration/unregistration failures | `rrdInterface.c` | RBUS mock failure injection |
| **P3** | Message queue creation failure | `rrdMain.c` | System resource exhaustion mock |
| **P3** | Event thread creation failure | `rrdMain.c` | Thread creation failure mock |
| **P3** | Directory creation/chdir failures | `rrdJsonParser.c`, `rrdRunCmdThread.c` | Filesystem permission mock |
| **P3** | systemd-run / journalctl execution failures | `rrdRunCmdThread.c` | Binary removal or mock failure |
| **P3** | Output file write errors | `rrdRunCmdThread.c` | Filesystem full or permission mock |
| **P3** | Dynamic profile JSON parse failure | `rrdDynamic.c` | Corrupted dynamic JSON file |
| **P3** | Invalid deep sleep event type | `rrdDynamic.c` | IARM event simulation with bad type |
| **P3** | Memory allocation failures (all modules) | All `.c` files | malloc failure injection (not practical in L2) |

---

## 5. Gap Summary

### 5.1 Feature vs Test Gap Table

| Behavior Area | Feature Scenarios | Test Functions | Coverage | Top Gaps |
|---|:---:|:---:|:---:|---|
| Daemon startup/subscribe | 1 | 4 | 100% | — |
| RFC enable/disable | 2 | 1 | 100% | — |
| Single instance | 1 | 3 | 100% | — |
| Static profile report | 5 | 5 | 100% | — |
| Static category report | 5 | 5 | 100% | — |
| Static suffix report | 4 | 5 | 100% | — |
| Static suffix negative | 4 | 5 | 100% | — |
| Background command | 5 | 5 | 100% | — |
| Dynamic profile report | 5 | 9 | 100% | — |
| Dynamic subcategory | 5 | 7 | 100% | — |
| Dynamic missing | 4 | 7 | 100% | — |
| Append mode | 4 | 7 | 100% | — |
| Append (static not found) | 4 | 7 | 100% | — |
| Harmful static | 5 | 5 | 100% | — |
| Harmful dynamic | 5 | 7 | 100% | — |
| Corrupted profile | 4 | 4 | 100% | — |
| Missing command | 5 | 5 | 100% | — |
| Empty issuetype | 2 | 2 | 100% | — |
| Deep sleep | 2 | 5 | 100% | — |
| Debug report upload | 6 | 5 | 100% | — |
| **C API upload** | **21** | **5** | **~24%** | **16 scenarios not implemented** |
| **Profile data SET/GET** | **0** | **3** | **N/A** | **No feature file** |
| **WebCfg event** | **0** | **0** | **0%** | **Entire flow untested** |
| **Upload lock contention** | **0** | **0** | **0%** | **No test** |
| **Config fallback chain** | **0** | **0** | **0%** | **No test** |

### 5.2 Action Items (Prioritized)

| # | Priority | Action | Effort |
|:---:|:---:|---|:---:|
| 1 | P1 | Implement 16 missing `test_rrd_c_api_upload.py` test functions matching feature scenarios | High |
| 2 | P1 | Create `rrd_profile_data.feature` for the existing `test_rrd_profile_data.py` test | Low |
| 3 | P1 | Add WebCfg event L2 test (`test_rrd_webcfg_event.py` + `rrd_webcfg_event.feature`) | High |
| 4 | P2 | Add upload lock contention test | Medium |
| 5 | P2 | Add configuration fallback chain test | Medium |
| 6 | P2 | Add archive CPU throttle test | Medium |
| 7 | P2 | Add dynamic profile JSON parse failure test | Low |
| 8 | P3 | Add RBUS registration failure test | Low |
| 9 | P3 | Add systemd-run / journalctl failure test | Low |
| 10 | P3 | Add output file write error test | Low |

---

## 6. Test Infrastructure Notes

### Test Interfaces

| Interface | Tool | Used By |
|---|---|---|
| RBUS event trigger | `rbuscli set` | All event-based tests |
| RBUS profile data | `rbuscli set/get` | `test_rrd_profile_data.py` |
| Log scraping | `grep_rrdlogs()` in `helper_functions.py` | All tests |
| File system checks | `os.path.isfile()`, `os.path.isdir()` | Prerequisite tests |
| Process control | `pidof`, `kill -9`, `nohup` | Setup/teardown |
| Mock upload server | Mock xconf server | `test_rrd_debug_report_upload.py` |
| Mock upload script | `uploadSTBLogs.sh` | Upload tests |

### Test Execution

- **Runner:** `pytest`, sequential per file
- **No ordering decorators:** Tests rely on file-level sequential execution order
- **Shared state:** Tests within a file depend on prior test side effects (daemon start → trigger → validate)
- **Cleanup:** Most files kill the daemon and remove logs in setup

---

## 7. Appendix: Complete File Inventory

### Feature Files (21)

| # | File | Scenarios |
|:---:|---|:---:|
| 1 | `rrd_start_subscribe_and_wait.feature` | 1 |
| 2 | `rrd_start_control.feature` | 2 |
| 3 | `rrd_single_instance.feature` | 1 |
| 4 | `rrd_static_profile_report.feature` | 5 |
| 5 | `rrd_static_profile_category_report.feature` | 5 |
| 6 | `test_rrd_static_profile_report_with_suffix.feature` | 4 |
| 7 | `test_rrd_static_profile_report_with_suffix_negative_case.feature` | 4 |
| 8 | `rrd_background_cmd_static_profile_report.feature` | 5 |
| 9 | `rrd_dynamic_profile_report.feature` | 5 |
| 10 | `rrd_dynamic_profile_subcategory_report.feature` | 5 |
| 11 | `rrd_dynamic_profile_missing_report.feature` | 4 |
| 12 | `rrd_append_report.feature` | 4 |
| 13 | `rrd_append_dynamic_profile_static_not_found.feature` | 4 |
| 14 | `rrd_harmful_command_static_report.feature` | 5 |
| 15 | `test_rrd_dynamic_profile_harmful_report.feature` | 5 |
| 16 | `rrd_corrupted_static_profile_report.feature` | 4 |
| 17 | `rrd_static_profile_missing_command_report.feature` | 5 |
| 18 | `rrd_empty_issuetype_event.feature` | 2 |
| 19 | `rrd_deepsleep_static_report.feature` | 2 |
| 20 | `rrd_debug_report_upload.feature` | 6 |
| 21 | `rrd_c_api_upload.feature` | 21 |
| | **Total** | **97** |

### Test Files (22)

| # | File | Test Functions |
|:---:|---|:---:|
| 1 | `test_rrd_start_subscribe_and_wait.py` | 4 |
| 2 | `test_rrd_start_control.py` | 1 |
| 3 | `test_rrd_single_instance.py` | 3 |
| 4 | `test_rrd_static_profile_report.py` | 5 |
| 5 | `test_rrd_static_profile_category_report.py` | 5 |
| 6 | `test_rrd_static_profile_report_with_suffix.py` | 5 |
| 7 | `test_rrd_static_profile_report_with_suffix_negative_case.py` | 5 |
| 8 | `test_rrd_background_cmd_static_profile_report.py` | 5 |
| 9 | `test_rrd_dynamic_profile_report.py` | 9 |
| 10 | `test_rrd_dynamic_subcategory_report.py` | 7 |
| 11 | `test_rrd_dynamic_profile_missing_report.py` | 7 |
| 12 | `test_rrd_append_report.py` | 7 |
| 13 | `test_rrd_append_dynamic_profile_static_notfound.py` | 7 |
| 14 | `test_rrd_harmful_command_static_report.py` | 5 |
| 15 | `test_rrd_dynamic_profile_harmful_report.py` | 7 |
| 16 | `test_rrd_corrupted_static_profile_report.py` | 4 |
| 17 | `test_rrd_static_profile_missing_command_report.py` | 5 |
| 18 | `test_rrd_empty_issuetype_event.py` | 2 |
| 19 | `test_rrd_deepsleep_static_report.py` | 5 |
| 20 | `test_rrd_debug_report_upload.py` | 5 |
| 21 | `test_rrd_c_api_upload.py` | 5 |
| 22 | `test_rrd_profile_data.py` | 3 |
| | **Total** | **112** |
