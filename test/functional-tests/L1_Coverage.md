# Remote Debugger L1 Unit-Test Coverage

**Generated:** 2026-10-23  
**Coverage source:** lcov/gcov from the L1 Google Test run  
**Scope:** RRD C source files compiled into `remotedebugger_gtest`

## Coverage Summary

| Metric | Coverage | Hit / total |
|---|:---:|:---:|
| Lines | **68.2%** | 1947 / 2856 |
| Functions | **92.7%** | 114 / 123 |
| Branches | **N/A** | 0 / 0 |

## L1 Test Results

| Metric | Count |
|---|---:|
| Test suites | 81 |
| Test cases | 336 |
| Failures | 0 |
| Disabled | 0 |
| Skipped | 0 |

## Per-Module Coverage

| Source module | Lines | Functions | Branches |
|---|:---:|:---:|:---:|
| `rrdCommandSanity.c` | 94.9% (74/78) | 100.0% (3/3) | N/A (0/0) |
| `rrdDynamic.c` | 85.2% (127/149) | 100.0% (5/5) | N/A (0/0) |
| `rrdEventProcess.c` | 60.6% (218/360) | 90.9% (10/11) | N/A (0/0) |
| `rrdExecuteScript.c` | 100.0% (15/15) | 100.0% (2/2) | N/A (0/0) |
| `rrdIarmEvents.c` | 88.2% (120/136) | 100.0% (4/4) | N/A (0/0) |
| `rrdInterface.c` | 72.4% (207/286) | 94.7% (18/19) | N/A (0/0) |
| `rrdJsonParser.c` | 55.4% (277/500) | 86.7% (13/15) | N/A (0/0) |
| `rrdMain.c` | 50.0% (23/46) | 33.3% (1/3) | N/A (0/0) |
| `rrdMsgPackDecoder.c` | 71.7% (198/276) | 100.0% (15/15) | N/A (0/0) |
| `rrdRunCmdThread.c` | 79.9% (163/204) | 100.0% (10/10) | N/A (0/0) |
| `rrd_archive.c` | 65.9% (166/252) | 100.0% (11/11) | N/A (0/0) |
| `rrd_config.c` | 47.5% (105/221) | 77.8% (7/9) | N/A (0/0) |
| `rrd_logproc.c` | 86.3% (63/73) | 100.0% (4/4) | N/A (0/0) |
| `rrd_sysinfo.c` | 79.8% (87/109) | 100.0% (6/6) | N/A (0/0) |
| `rrd_upload.c` | 67.8% (61/90) | 80.0% (4/5) | N/A (0/0) |
| `uploadRRDLogs.c` | 70.5% (43/61) | 100.0% (1/1) | N/A (0/0) |

## Test Suites

| Suite | Tests | Failures | Disabled | Skipped |
|---|---:|---:|---:|---:|
| `GetParamCountTest` | 4 | 0 | 0 | 0 |
| `RemoveSpecialCharTest` | 3 | 0 | 0 | 0 |
| `ReadJsonFileTest` | 3 | 0 | 0 | 0 |
| `ReadAndParseJSONTest` | 3 | 0 | 0 | 0 |
| `GetIssueInfoTest` | 2 | 0 | 0 | 0 |
| `FindIssueInParsedJSONTest` | 5 | 0 | 0 | 0 |
| `InvokeSanityandCommandExecTest` | 5 | 0 | 0 | 0 |
| `ProcessAllDebugCommandTest` | 4 | 0 | 0 | 0 |
| `ProcessAllDeepSleepAwkMetricsCommandsTest` | 3 | 0 | 0 | 0 |
| `UpdateBackgroundCmdTest` | 3 | 0 | 0 | 0 |
| `IsCommandsValidTest` | 2 | 0 | 0 | 0 |
| `ReplaceRRDLocationTest` | 2 | 0 | 0 | 0 |
| `SetParamByRFC` | 1 | 0 | 0 | 0 |
| `IARMBusTest` | 3 | 0 | 0 | 0 |
| `RBusApiTest` | 1 | 0 | 0 | 0 |
| `WebConfigTest` | 1 | 0 | 0 | 0 |
| `RRDCheckIssueInDynamicProfileTest` | 5 | 0 | 0 | 0 |
| `RRDRdmManagerDownloadRequestTest` | 6 | 0 | 0 | 0 |
| `RRDProcessDeepSleepAwakeEventsTest` | 6 | 0 | 0 | 0 |
| `NormalizeIssueNameTest` | 5 | 0 | 0 | 0 |
| `UploadDebugoutputTest` | 2 | 0 | 0 | 0 |
| `InitCacheTest` | 1 | 0 | 0 | 0 |
| `PrintItemsTest` | 2 | 0 | 0 | 0 |
| `CreateCacheTest` | 4 | 0 | 0 | 0 |
| `AppendItemTest` | 8 | 0 | 0 | 0 |
| `FreecacheDataCacheNodeTest` | 3 | 0 | 0 | 0 |
| `RemoveQuotesTest` | 6 | 0 | 0 | 0 |
| `CopyDebugLogDestFileTest` | 2 | 0 | 0 | 0 |
| `FindPresentInCacheTest` | 4 | 0 | 0 | 0 |
| `RemoveItemTest` | 3 | 0 | 0 | 0 |
| `FreeParsedJsonTest` | 2 | 0 | 0 | 0 |
| `RemoveSpecialCharacterfromIssueTypeListTest` | 4 | 0 | 0 | 0 |
| `IssueTypeSplitterTest` | 3 | 0 | 0 | 0 |
| `SplitIssueTypeTest` | 22 | 0 | 0 | 0 |
| `ProcessIssueTypeInDynamicProfileTest` | 3 | 0 | 0 | 0 |
| `ProcessWebCfgTypeEvntTest` | 2 | 0 | 0 | 0 |
| `ProcessIssueTypeEvntTest` | 8 | 0 | 0 | 0 |
| `ProcessIssueTypeInInstalledPackageTest` | 1 | 0 | 0 | 0 |
| `processIssueTypeTest` | 2 | 0 | 0 | 0 |
| `SetBlobVersionTest` | 1 | 0 | 0 | 0 |
| `GetBlobVersionTest` | 1 | 0 | 0 | 0 |
| `RRDDataBuffInitTest` | 2 | 0 | 0 | 0 |
| `RRDDataBuffDeAllocTest` | 4 | 0 | 0 | 0 |
| `RRDUnsubscribeTest` | 6 | 0 | 0 | 0 |
| `WebConfigIntegrationTest` | 1 | 0 | 0 | 0 |
| `RRDMsgDeliverTest` | 2 | 0 | 0 | 0 |
| `PushIssueTypesToMsgQueueTest` | 1 | 0 | 0 | 0 |
| `RDMMgrEventHandlerTest` | 6 | 0 | 0 | 0 |
| `PwrMgrEventHandlerTest` | 5 | 0 | 0 | 0 |
| `RRDSubscribeTest` | 5 | 0 | 0 | 0 |
| `TestDebuggerRollback` | 1 | 0 | 0 | 0 |
| `TestRRDDocDestroy` | 4 | 0 | 0 | 0 |
| `FinderTest` | 5 | 0 | 0 | 0 |
| `GetMsgpackUnpackStatusTest` | 4 | 0 | 0 | 0 |
| `RemoteDebuggerDocStrErrorTest` | 5 | 0 | 0 | 0 |
| `GetReqTotalSizeOfIssueTypesStrTest` | 3 | 0 | 0 | 0 |
| `FreeResourcesRemoteDebuggerTest` | 1 | 0 | 0 | 0 |
| `PrepareDataToPushTest` | 1 | 0 | 0 | 0 |
| `HelperConvertTest` | 5 | 0 | 0 | 0 |
| `ProcessRemoteDebuggerParamsTest` | 5 | 0 | 0 | 0 |
| `ProcessRRDWfgReq` | 2 | 0 | 0 | 0 |
| `Base64DecodeTest` | 7 | 0 | 0 | 0 |
| `RemoteDebuggerDocTest` | 2 | 0 | 0 | 0 |
| `RemoteDebuggerDocConvertTest` | 1 | 0 | 0 | 0 |
| `DecodeWebCfgDataTest` | 1 | 0 | 0 | 0 |
| `RRDEventThreadFuncTest` | 5 | 0 | 0 | 0 |
| `LookupRrdProfileListTest` | 3 | 0 | 0 | 0 |
| `ExecuteCommandsTest` | 1 | 0 | 0 | 0 |
| `CheckAppendRequestTest` | 5 | 0 | 0 | 0 |
| `GetIssueCommandInfoTest` | 2 | 0 | 0 | 0 |
| `RRDUploadOrchestrationTest` | 46 | 0 | 0 | 0 |
| `LoadProfileCategoryTest` | 4 | 0 | 0 | 0 |
| `SaveProfileCategoryTest` | 2 | 0 | 0 | 0 |
| `HasDirectCommandsTest` | 4 | 0 | 0 | 0 |
| `ReadProfileJsonFileTest` | 4 | 0 | 0 | 0 |
| `GetAllCategoriesJsonTest` | 3 | 0 | 0 | 0 |
| `GetSpecificCategoryJsonTest` | 3 | 0 | 0 | 0 |
| `RrdSetHandlerTest` | 1 | 0 | 0 | 0 |
| `RrdGetHandlerTest` | 1 | 0 | 0 | 0 |
| `SetRbusResponseTest` | 2 | 0 | 0 | 0 |
| `RRDProfileHandlerTest` | 20 | 0 | 0 | 0 |

## Uncovered Functions

These functions were present in the instrumented RRD source but had no recorded call during this L1 run.

| Source module | Function |
|---|---|
| `rrdEventProcess.c` | `processIssueTypeInStaticProfileappend` |
| `rrdInterface.c` | `RRD_subscribe` |
| `rrdJsonParser.c` | `_Z18RRDStoreDeviceInfoP16deviceProperties` |
| `rrdJsonParser.c` | `checkIssueNodeInfo` |
| `rrdMain.c` | `_Z10shadowMainPv` |
| `rrdMain.c` | `_Z12isRRDEnabledv` |
| `rrd_config.c` | `_Z20rrd_config_get_valuePK12rrd_config_tPKc` |
| `rrd_config.c` | `_ZL15execute_commandPKcPcm` |
| `rrd_upload.c` | `rrd_upload_cleanup_files` |

## Coverage Follow-Up

Use the lowest-coverage modules and uncovered-function list to prioritize focused unit tests. Coverage indicates execution, not assertion quality or complete behavior validation.
