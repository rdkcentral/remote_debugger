---
name: bdd-feature-generator
description: Generate BDD (Behavior Driven Development) feature files from remote debugger source code analysis. Use for creating Gherkin-format documentation of RBUS event handling, static/dynamic profile processing, command execution, harmful command detection, log archive and upload, deep sleep handling, and uploadRRDLogs C API orchestration. Produces gap analysis between feature files and L2 test implementations.
---

# BDD Feature Generator for Remote Debugger

## Purpose

Automatically generate BDD feature files in Gherkin format by analyzing the remote debugger source code. This skill creates comprehensive behavioral documentation that can serve as:
- **Functional documentation** of daemon lifecycle, event handling, and profile processing
- **Test specifications** for L2 functional tests (`test/functional-tests/`)
- **Requirements traceability** linking source modules to observable behavior
- **Gap analysis baseline** for comparing L2 tests vs implemented behavior

## Usage

Invoke this skill when:
- Documenting existing remote debugger behaviors in BDD format
- Creating test specifications for new features (e.g., new profile types, upload mechanisms)
- Generating feature files for untested code paths (e.g., WebCfg events, deep sleep edge cases)
- Performing gap analysis between L2 tests and source implementation
- Onboarding new team members with behavioral documentation of the daemon

## Project Context

The remote debugger (`remotedebugger`) is an RDK component that enables remote collection, packaging, and upload of device diagnostic logs. It:
- Listens for **RBUS/TR-181 events** (IssueType triggers) to initiate debug data collection
- Processes **static profiles** (built-in JSON config at `/etc/rrd/remote_debugger.json`) and **dynamic profiles** (downloaded packages at `/media/apps/RDK-RRD-<Node>/etc/rrd/remote_debugger.json`)
- Executes diagnostic **commands** from profiles, with **sanity checks** against harmful commands
- Archives collected outputs as `.tgz` and **uploads** them to a remote server via `uploadRRDLogs.sh` or the **C API** (`rrd_upload_orchestrate`)
- Supports **append mode** combining commands from both static and dynamic profiles
- Handles **deep sleep** events for deferred processing
- Supports **category-only** issue types (all sub-nodes under a category)
- Enforces **single-instance** execution and **RFC enable/disable** control
- Runs as a **systemd service** with a main thread and a dedicated event-processing thread

## Prerequisites

Before running this skill:

1. **Review the build system** — `Makefile.am`, `src/Makefile.am`, and `configure.ac`
2. **Identify compiled modules** — Core sources are always compiled; IARMBUS sources are conditional
3. **Review existing feature files** — Match the format in `test/functional-tests/features/`
4. **Understand test interfaces** — L2 tests use `rbuscli` (RBUS event trigger), log scraping (`/opt/logs/remotedebugger.log.0`), and file system checks (`/tmp/rrd/`)

## Process

### Step 1: Analyze Build Configuration

The remote debugger build is Autotools-based. Identify compiled components from the Makefile chain:

```bash
# Top-level: identifies src/ as the main SUBDIR
cat Makefile.am | grep "SUBDIRS"
# → SUBDIRS = src

# Source level: identifies the remotedebugger binary and its sources
cat src/Makefile.am
# → bin_PROGRAMS = remotedebugger
# → remotedebugger_SOURCES = rrdMain.c rrdEventProcess.c rrdJsonParser.c
#     rrdRunCmdThread.c rrdCommandSanity.c rrdDynamic.c rrdExecuteScript.c
#     rrdMsgPackDecoder.c rrdInterface.c
# → if IARMBUS_ENABLE:
#     remotedebugger_SOURCES += rrdIarmEvents.c uploadRRDLogs.c rrd_config.c
#       rrd_sysinfo.c rrd_logproc.c rrd_archive.c rrd_upload.c
```

**Always compiled modules:**

| Source File | Header | Purpose |
|---|---|---|
| `rrdMain.c` | `rrdMain.h` | Daemon entry, event thread, RFC enable check, message queue setup |
| `rrdEventProcess.c` | `rrdEventProcess.h` | IssueType/WebCfg/DeepSleep event dispatch, static/dynamic profile flow |
| `rrdJsonParser.c` | `rrdJsonParser.h` | JSON profile parsing, command/timeout extraction, issue node lookup |
| `rrdRunCmdThread.c` | `rrdRunCmdThread.h` | Command execution, output file management, result caching |
| `rrdCommandSanity.c` | `rrdCommandSanity.h` | Command validation against harmful command blocklist |
| `rrdDynamic.c` | `rrdDynamic.h` | Dynamic profile handling, deep sleep event processing |
| `rrdExecuteScript.c` | `rrdExecuteScript.h` | Upload debug output orchestration |
| `rrdMsgPackDecoder.c` | `rrdMsgPackDecoder.h` | WebConfig parameter decoding (MsgPack format) |
| `rrdInterface.c` | `rrdInterface.h` | RBUS registration, event handler setup, profile data elements |

**Conditionally compiled modules (IARMBUS_ENABLE):**

| Source File | Header | Purpose |
|---|---|---|
| `rrdIarmEvents.c` | — | IARM bus event handling (power state, deep sleep) |
| `uploadRRDLogs.c` | — | C implementation of upload orchestration entry point |
| `rrd_config.c` | `rrd_config.h` | Configuration loading (server URLs, paths, protocol, RFC/DCM) |
| `rrd_sysinfo.c` | `rrd_sysinfo.h` | System info (MAC address, timestamp, file/dir checks) |
| `rrd_logproc.c` | `rrd_logproc.h` | Log directory validation, preparation, live log handling |
| `rrd_archive.c` | `rrd_archive.h` | Archive creation (.tgz), filename generation, cleanup, CPU checks |
| `rrd_upload.c` | `rrd_upload.h` | Upload orchestration, lock handling, cleanup |

**Shared headers (no implementation file):**

| Header | Purpose |
|---|---|
| `rrdCommon.h` | Common constants, macros, data structures (data_buf, msgRRDHdr, etc.) |
| `rrdRbus.h` | RBUS event subscription, blob versioning |
| `rrd_log.h` | Logging subsystem initialization |

**Exclude from feature generation:**
- `src/unittest/` — Unit tests (L1)
- `test/` — Test infrastructure
- `scripts/` — Runtime scripts (documented as behavior, not source)
- `docs/` — Existing documentation

### Step 2: Analyze Source Code Structure

For each compiled module:

1. **Read the header file** (`.h`) — Identify public functions, data structures, constants
2. **Read the implementation** (`.c`) — Extract event flows, log messages, error paths
3. **Identify RBUS parameters/events** — Map registered data elements in `rrdInterface.c`
4. **Note conditional compilation** — `#ifdef IARMBUS_SUPPORT` gates upload and IARM modules
5. **Note key log messages** — Tests validate behavior by grepping these from log files
6. **Note file paths** — `/etc/rrd/remote_debugger.json`, `/tmp/rrd/`, `/media/apps/RDK-RRD-*`

**Key elements to extract for each module:**

| Element | Where to Find | Example |
|---|---|---|
| RBUS data elements | `rrdInterface.c` registration | `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType` |
| Event handlers | `rrdInterface.c` set handlers | `RRD_SET_ISSUE_EVENT` |
| Profile JSON paths | `rrdEventProcess.c`, `rrdDynamic.c` | `/etc/rrd/remote_debugger.json`, `/media/apps/RDK-RRD-{Node}/etc/rrd/remote_debugger.json` |
| Log messages | All `.c` files `RDK_LOG_*` calls | `"SUCCESS: Message sending Done"`, `"Json File parse Success..."` |
| Error conditions | `.c` return/log on failure | `"FAILED: Json File parse..."`, `"Harmful Command Found"` |
| File outputs | `rrdRunCmdThread.c` | `/tmp/rrd/{IssueType}/debug_outputs.txt` |
| Archive naming | `rrd_archive.c` | `{MAC}_{ISSUETYPE}_{TIMESTAMP}_RRD_DEBUG_LOGS.tgz` |
| Upload API | `rrd_upload.h` | `rrd_upload_orchestrate(upload_dir, issue_type)` |
| Compile guards | `.c` / `.h` `#ifdef` | `IARMBUS_SUPPORT` |

### Step 3: Create Feature File Structure

Feature files are placed in `test/functional-tests/features/`.

**Naming convention for remote debugger:**

| Behavior Area | Feature File | Description |
|---|---|---|
| Daemon startup | `rrd_start_subscribe_and_wait.feature` | RBUS subscription, event loop entry |
| RFC enable/disable | `rrd_start_control.feature` | RFC-controlled start/stop |
| Single instance | `rrd_single_instance.feature` | Prevents duplicate daemon instances |
| Static profile processing | `rrd_static_profile_report.feature` | End-to-end static profile flow |
| Static category report | `rrd_static_profile_category_report.feature` | Category-only issue type |
| Static profile with suffix | `test_rrd_static_profile_report_with_suffix.feature` | Suffixed issue type handling |
| Background command | `rrd_background_cmd_static_profile_report.feature` | Background command execution |
| Dynamic profile processing | `rrd_dynamic_profile_report.feature` | Dynamic profile fallback flow |
| Dynamic subcategory | `rrd_dynamic_profile_subcategory_report.feature` | Dynamic profile subcategory |
| Dynamic profile missing | `rrd_dynamic_profile_missing_report.feature` | Missing dynamic profile |
| Append mode | `rrd_append_report.feature` | Static+dynamic profile append |
| Append (static not found) | `rrd_append_dynamic_profile_static_not_found.feature` | Append when static missing |
| Harmful commands | `rrd_harmful_command_static_report.feature` | Sanity check blocks execution |
| Dynamic harmful | `test_rrd_dynamic_profile_harmful_report.feature` | Harmful command in dynamic profile |
| Corrupted profile | `rrd_corrupted_static_profile_report.feature` | Invalid/corrupted JSON handling |
| Static missing command | `rrd_static_profile_missing_command_report.feature` | Missing command in profile |
| Empty issue type | `rrd_empty_issuetype_event.feature` | Empty event value handling |
| Deep sleep | `rrd_deepsleep_static_report.feature` | Deep sleep issue type handling |
| Debug report upload | `rrd_debug_report_upload.feature` | Full upload + download validation |
| C API upload | `rrd_c_api_upload.feature` | `rrd_upload_orchestrate` C API tests |
| Suffix negative case | `test_rrd_static_profile_report_with_suffix_negative_case.feature` | Invalid suffix handling |

### Step 4: Generate Feature Files

Use this template for remote debugger feature files:

```gherkin
##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright [YEAR] RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

# Source: src/{SourceFile}.c

Feature: Remote Debugger {Behavior Description}

  Scenario: {Prerequisite Check}
    Given the configuration file path is set
    When I check if the configuration file exists
    Then the configuration file should exist

  Scenario: Verify remote debugger process is running
    Given the remote debugger process is not running
    When I start the remote debugger process
    Then the remote debugger process should be running

  Scenario: {Main Behavior Scenario}
    Given the remote debugger is running
    When I trigger the event "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType"
    Then {expected behavior validated via log scraping}
```

### Step 5: Map Source Code to Scenarios

**For each remote debugger behavior, create scenarios covering:**

1. **Prerequisites** — Config file exists, output directory exists, daemon not already running
2. **Daemon startup** — Process starts, RBUS subscription succeeds, event loop enters wait
3. **Event trigger** — RBUS IssueType set via `rbuscli`, message queue send/receive
4. **Profile parsing** — JSON read, parse success/failure, issue node lookup
5. **Command execution** — Sanity check, command run, output file creation
6. **Service management** — systemd service start/stop, journalctl collection
7. **Upload flow** — Archive creation, upload script invocation, success/failure
8. **Error/edge cases** — Harmful commands, empty events, corrupted profiles, missing profiles

**Example mapping — RBUS event trigger (rrdInterface.c → rrdEventProcess.c):**

```c
// Source: src/rrdInterface.c — RBUS set handler for IssueType
// Sends message to event thread via message queue
// Source: src/rrdEventProcess.c — Event thread receives and dispatches
```

**Generated scenarios:**

```gherkin
Scenario: Send WebPA event for IssueType and verify message flow
  Given the remote debugger is running
  When I trigger the event "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType"
  Then the event for RRD_SET_ISSUE_EVENT should be received
  And the logs should contain "SUCCESS: Message sending Done"
  And the logs should be seen with "SUCCESS: Message Reception Done"
```

**Example mapping — Static profile processing (rrdEventProcess.c → rrdJsonParser.c):**

```gherkin
Scenario: Process static profile for issue type
  When the remotedebugger received the message from webPA event
  Then remotedebugger should read the Json file
  And remotedebugger logs should contain the Json File Parse success
  And the issue data node and sub-node should be found in the JSON file
  And the directory should be created to store the executed output
  And Sanity check to validate the commands should be executed
  And Command output should be added to the output file
```

**Example mapping — Dynamic profile fallback (rrdDynamic.c):**

```gherkin
Scenario: Verify the Issuetype in dynamic path
  Given the remote debugger issuetype is missing in static profile
  When the remotedebugger read the json file form the dynamic path
  Then remotedebugger json read and parse should be success
  And remotedebugger should read the Issuetype from dynamic profile
  And the issue data node and sub-node should be found in the JSON file
```

**Example mapping — Harmful command detection (rrdCommandSanity.c):**

```gherkin
Scenario: Check for harmful commands and abort
  Given remote debugger parse the static json profile successfully
  When the issue command and the sanity commands are matched
  Then the remote debugger should exit the processing of commands
  And Abort the command execution and skip report upload
```

**Example mapping — Upload orchestration C API (rrd_upload.c):**

```gherkin
Scenario: Validate rrd_upload_orchestrate C API with valid parameters
  Given the remote debugger is configured
  And test log files are created in the upload directory
  When I call rrd_upload_orchestrate with valid upload directory and issue type
  Then the C API should return success code 0
  And logs should contain "Configuration loaded"
  And logs should contain "MAC:" for MAC address
  And logs should contain "Log directory validated and prepared"
  And logs should contain "Issue type sanitized"
  And logs should contain "Archive filename:" with the generated filename
  And logs should contain "Creating" for tarfile creation
  And logs should contain "Invoking uploadSTBLogs binary to upload"
```

**Example mapping — Single instance enforcement (rrdMain.c):**

```gherkin
Scenario: Remote debugger exits if another instance is invoked
  Given the RemoteDebugger is not already running
  When the RemoteDebugger binary is invoked
  Then the RemoteDebugger should be started
  And when the RemoteDebugger is attempted to be started again
  Then the RemoteDebugger should not start another instance
```

### Step 6: Document Issue Type Variations

For issue types with multiple forms, use individual scenarios:

```gherkin
Scenario: Static profile with Node.SubNode issue type
  Given the remote debugger is running
  When I trigger IssueType "Device.Info"
  Then the issue data node "Device" and sub-node "Info" should be found in the JSON file

Scenario: Category-only issue type (all sub-nodes)
  Given the remote debugger is running
  When I trigger IssueType "Device"
  Then all sub-nodes under category "Device" should be processed

Scenario: Suffixed issue type
  Given the remote debugger is running
  When I trigger IssueType "Device.Info_ab1bghjh"
  Then the base issue "Device.Info" should be matched in the profile
  And the archive filename should include the full suffixed issue type
```

### Step 7: Create README Index

Create or update `test/functional-tests/features/README.md`:

```markdown
# Remote Debugger Feature Documentation

This folder contains BDD feature files documenting the remote debugger
daemon behavior implemented in `src/`.

## Feature Files Overview

| Feature File | Source Components | Description |
|---|---|---|
| `rrd_start_subscribe_and_wait.feature` | `rrdMain.c`, `rrdInterface.c` | RBUS subscription, event loop |
| `rrd_start_control.feature` | `rrdMain.c` | RFC enable/disable control |
| `rrd_single_instance.feature` | `rrdMain.c` | Single instance enforcement |
| `rrd_static_profile_report.feature` | `rrdEventProcess.c`, `rrdJsonParser.c`, `rrdRunCmdThread.c` | Static profile end-to-end |
| `rrd_static_profile_category_report.feature` | `rrdEventProcess.c`, `rrdJsonParser.c` | Category-only issue type |
| `rrd_dynamic_profile_report.feature` | `rrdDynamic.c`, `rrdJsonParser.c` | Dynamic profile fallback |
| `rrd_dynamic_profile_subcategory_report.feature` | `rrdDynamic.c` | Dynamic subcategory |
| `rrd_append_report.feature` | `rrdDynamic.c`, `rrdEventProcess.c` | Append mode (static+dynamic) |
| `rrd_harmful_command_static_report.feature` | `rrdCommandSanity.c` | Harmful command blocking |
| `rrd_corrupted_static_profile_report.feature` | `rrdJsonParser.c` | Corrupted JSON handling |
| `rrd_empty_issuetype_event.feature` | `rrdEventProcess.c` | Empty event value |
| `rrd_deepsleep_static_report.feature` | `rrdDynamic.c`, `rrdIarmEvents.c` | Deep sleep handling |
| `rrd_debug_report_upload.feature` | `rrdExecuteScript.c`, `uploadRRDLogs.sh` | Upload + download validation |
| `rrd_c_api_upload.feature` | `rrd_upload.c`, `rrd_config.c`, `rrd_archive.c` | C API upload orchestration |

## Source Module Mapping

Based on `src/Makefile.am` and `configure.ac`:

### Always Compiled
- `rrdMain.c` — Daemon lifecycle, event thread, RFC enable check
- `rrdEventProcess.c` — IssueType/WebCfg event dispatch
- `rrdJsonParser.c` — JSON profile parsing, command extraction
- `rrdRunCmdThread.c` — Command execution, output management
- `rrdCommandSanity.c` — Harmful command validation
- `rrdDynamic.c` — Dynamic profile handling, deep sleep
- `rrdExecuteScript.c` — Upload debug output orchestration
- `rrdMsgPackDecoder.c` — WebConfig MsgPack decoding
- `rrdInterface.c` — RBUS registration, event handlers

### Conditionally Compiled (IARMBUS_ENABLE)
- `rrdIarmEvents.c` — IARM bus events (power state)
- `uploadRRDLogs.c` — C upload orchestration entry point
- `rrd_config.c` — Configuration loading (RFC/DCM/fallback)
- `rrd_sysinfo.c` — System info (MAC, timestamp)
- `rrd_logproc.c` — Log directory validation/preparation
- `rrd_archive.c` — Archive creation (.tgz)
- `rrd_upload.c` — Upload orchestration, lock handling

## RBUS Data Elements

| Data Element | Type | Handler |
|---|---|---|
| `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType` | SET event | Triggers debug data collection |
| `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.WebCfgData` | SET event | WebConfig-based trigger |
| `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.setProfileData` | SET | Profile data injection |
| `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.getProfileData` | GET | Profile data retrieval |

## Test Interface Summary

| Interface | Tool | Log File | Used For |
|---|---|---|---|
| RBUS event | `rbuscli set` | `/opt/logs/remotedebugger.log.0` | IssueType trigger |
| Log scraping | Python `grep_rrdlogs()` | `/opt/logs/remotedebugger.log.0` | Behavior validation |
| File system | Python `os.path`, `subprocess` | `/tmp/rrd/`, `/etc/rrd/` | Config/output checks |
| Upload | Mock xconf server | — | Archive upload validation |
| Process control | `pidof`, `kill`, `nohup` | — | Daemon start/stop |

## Generation Date

Generated: {DATE}
```

## Scenario Patterns for Remote Debugger

### Prerequisite Check Pattern

```gherkin
Scenario: Check if remote debugger configuration file exists
  Given the configuration file path is set
  When I check if the configuration file exists
  Then the configuration file should exist

Scenario: Check if /tmp/rrd output directory exists
  Given the /tmp/rrd directory path is set
  When I check if the /tmp/rrd directory exists
  Then the /tmp/rrd directory should exist
```

### Daemon Startup Pattern

```gherkin
Scenario: Verify remote debugger process is running
  Given the remote debugger process is not running
  When I start the remote debugger process
  Then the remote debugger process should be running

Scenario: Remote debugger should subscribe to events
  Given the remote debugger binary is invoked
  When the remote debugger is started
  Then the remote debugger should subscribe to rbus and wait for the events
  And the log file should contain "SUCCESS: RBUS Event Subscribe for RRD done!"
  And the log file should contain "Waiting for TR69/RBUS Events..."
```

### RFC Enable/Disable Pattern

```gherkin
Scenario: Remote Debugger Starts when Enabled
  Given RFC Value for RDKRemoteDebugger.Enable is enabled
  When the remotedebugger is started check the value of the RFC parameter
  And the RDKRemoteDebugger Enable value is true
  Then the remotedebugger should be started and running as daemon

Scenario: Remote Debugger Stops when Disabled
  Given RFC Value for RDKRemoteDebugger.Enable is disabled
  When the remotedebugger is started check the value of the RFC parameter
  And the RDKRemoteDebugger Enable value is false
  Then the remotedebugger must be stopped and process should not be running
```

### RBUS Event Trigger Pattern

```gherkin
Scenario: Send WebPA event for IssueType
  Given the remote debugger is running
  When I trigger the event "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType"
  Then the event for RRD_SET_ISSUE_EVENT should be received
  And the logs should contain "SUCCESS: Message sending Done"
  And the logs should be seen with "SUCCESS: Message Reception Done"
```

### Static Profile Processing Pattern

```gherkin
Scenario: Process static profile commands
  When the remotedebugger received the message from webPA event
  Then remotedebugger should read the Json file
  And remotedebugger logs should contain the Json File Parse success
  And the issue data node and sub-node should be found in the JSON file
  And the directory should be created to store the executed output
  And Sanity check to validate the commands should be executed
  And Command output should be added to the output file
  And the issuetype systemd service should start successfully
  And the journalctl service should start successfully
  And the process should sleep with timeout
  And the issuetype systemd service should stop successfully
  And the remotedebugger should call script to upload the debug report
```

### Dynamic Profile Fallback Pattern

```gherkin
Scenario: Verify the Issuetype is not found in static profile
  Given the remote debugger received the message from RBUS command
  When the remotedebugger static json profile is present
  Then remotedebugger should read the Json file
  And remotedebugger logs should contain the Json File Parse Success
  And remotedebugger should log as the Issue requested is not found in the profile

Scenario: Verify the Issuetype in dynamic path
  Given the remote debugger issuetype is missing in static profile
  When the remotedebugger read the json file from the dynamic path
  Then remotedebugger json read and parse should be success
  And remotedebugger should read the Issuetype from dynamic profile
  And the issue data node and sub-node should be found in the JSON file
```

### Append Mode Pattern

```gherkin
Scenario: Verify append of static and dynamic profile commands
  Given the remote debugger received the message from RBUS command
  When the remotedebugger read the json file from the dynamic path
  Then remotedebugger json read and parse should be success in dynamic path
  And remotedebugger should read the Json file
  And remotedebugger logs should contain the Json File Parse Success
  And remotedebugger should log as the Issue requested found in the profile
  And Update the command after appending data from both profiles, then execute
```

### Harmful Command Detection Pattern

```gherkin
Scenario: Check for harmful commands and abort
  Given remote debugger parse the static json profile successfully
  When the issue node and subnode are present in the profile
  Then the remote debugger should read the Sanity Check list from profile
  And the remotedebugger should perform sanity check on issue commands
  Given the remote debugger profile has the harmful commands
  When the issue command and the sanity commands are matched
  Then the remote debugger should exit the processing of commands
  And Abort the command execution and skip report upload
```

### Upload Flow Pattern

```gherkin
Scenario: Upload remote debugger debug report
  Given the remote debugger completed the command execution
  When remotedebugger calls the uploadRRD.sh script
  Then check for the tarfile is created in the output directory
  And the file is uploaded to the mockxconf server
  And the upload success logs are seen in the logs

Scenario: Download the file from the mockxconf server
  Given the remote debugger report upload success
  When curl command is used to download the file
  Then the curl command should return success
  And the file should be downloaded successfully
```

### C API Upload Orchestration Pattern

```gherkin
Scenario: Validate rrd_upload_orchestrate with valid parameters
  Given the remote debugger is configured
  And test log files are created in the upload directory
  When I call rrd_upload_orchestrate with valid upload directory and issue type
  Then the C API should return success code 0
  And logs should contain "Configuration loaded"
  And logs should contain "MAC:" for MAC address
  And logs should contain "Log directory validated and prepared"
  And logs should contain "Issue type sanitized"
  And logs should contain "Archive filename:" with the generated filename
  And logs should contain "Invoking uploadSTBLogs binary to upload"

Scenario: Test rrd_upload_orchestrate with NULL parameters
  Given the remote debugger is configured
  When I call rrd_upload_orchestrate with NULL upload directory
  Then the C API should return error code 1
  And error logs should contain "Invalid parameters"

Scenario: Test rrd_upload_orchestrate with empty directory
  Given the remote debugger is configured
  And the upload directory is empty
  When I call rrd_upload_orchestrate with the empty directory
  Then the C API should return error code 6
  And error logs should contain "Invalid or empty upload directory"
```

### Error/Edge Case Pattern

```gherkin
Scenario: Corrupted JSON profile
  When the remotedebugger received the message from webPA event
  Then remotedebugger should read the Json file
  And remotedebugger logs should contain the Json File Parse Failed

Scenario: Empty issue type event
  Given the remote debugger is running
  When I trigger the event with empty IssueType value
  Then the remotedebugger receives the message from webPA event
  And remotedebugger should log as not processing empty event

Scenario: Missing issue type in static profile
  When the remotedebugger received the message
  Then remotedebugger should log as the Issue requested is not found in the profile
```

### Single Instance Pattern

```gherkin
Scenario: Remote debugger exits if another instance is invoked
  Given the RemoteDebugger is not already running
  When the RemoteDebugger binary is invoked
  Then the RemoteDebugger should be started
  And when the RemoteDebugger is attempted to be started again
  Then the RemoteDebugger should not start another instance
```

## Quality Checklist

Before completing feature generation for remote debugger:

- [ ] All source modules analyzed (`src/Makefile.am` sources list)
- [ ] Conditional modules noted with build flags (`IARMBUS_ENABLE`)
- [ ] Each RBUS event handler has at least one trigger scenario
- [ ] Static profile happy path has end-to-end scenario (trigger → parse → execute → upload)
- [ ] Dynamic profile fallback flow documented
- [ ] Append mode (static+dynamic) documented
- [ ] Harmful command sanity check scenarios included
- [ ] Corrupted/missing profile error scenarios documented
- [ ] Empty/invalid issue type edge cases documented
- [ ] Deep sleep event handling documented
- [ ] C API upload orchestration scenarios included (valid + error cases)
- [ ] RFC enable/disable control documented
- [ ] Single instance enforcement documented
- [ ] Suffixed issue type handling documented
- [ ] License headers included (Apache 2.0, RDK Management)
- [ ] Source file references included as comments
- [ ] Log message assertions match actual daemon log output
- [ ] Feature-to-test file mapping documented for gap analysis
- [ ] Scenarios are atomic (one behavior per scenario)
- [ ] Given/When/Then/And/Or structure followed consistently

## Output Structure

```
test/functional-tests/
├── features/
│   ├── README.md                                              # Index, module mapping, gap summary
│   ├── rrd_start_subscribe_and_wait.feature                   # RBUS subscription, event loop
│   ├── rrd_start_control.feature                              # RFC enable/disable
│   ├── rrd_single_instance.feature                            # Single instance enforcement
│   ├── rrd_static_profile_report.feature                      # Static profile end-to-end
│   ├── rrd_static_profile_category_report.feature             # Category-only issue type
│   ├── test_rrd_static_profile_report_with_suffix.feature     # Suffixed issue type
│   ├── test_rrd_static_profile_report_with_suffix_negative_case.feature  # Invalid suffix
│   ├── rrd_background_cmd_static_profile_report.feature       # Background command execution
│   ├── rrd_dynamic_profile_report.feature                     # Dynamic profile fallback
│   ├── rrd_dynamic_profile_subcategory_report.feature         # Dynamic subcategory
│   ├── rrd_dynamic_profile_missing_report.feature             # Missing dynamic profile
│   ├── rrd_append_report.feature                              # Append mode (static+dynamic)
│   ├── rrd_append_dynamic_profile_static_not_found.feature    # Append when static missing
│   ├── rrd_harmful_command_static_report.feature              # Harmful command blocking
│   ├── test_rrd_dynamic_profile_harmful_report.feature        # Dynamic harmful commands
│   ├── rrd_corrupted_static_profile_report.feature            # Invalid/corrupted JSON
│   ├── rrd_static_profile_missing_command_report.feature      # Missing command in profile
│   ├── rrd_empty_issuetype_event.feature                      # Empty event value
│   ├── rrd_deepsleep_static_report.feature                    # Deep sleep handling
│   ├── rrd_debug_report_upload.feature                        # Upload + download validation
│   └── rrd_c_api_upload.feature                               # C API upload orchestration
└── tests/
    ├── helper_functions.py                                     # Log grep, process control, constants
    ├── test_rrd_static_profile_report.py                      # Static profile test
    ├── test_rrd_dynamic_profile_report.py                     # Dynamic profile test
    ├── test_rrd_c_api_upload.py                               # C API upload test
    ├── test_rrd_single_instance.py                            # Single instance test
    ├── ...                                                    # (see full listing below)
    └── uploadSTBLogs.sh                                       # Mock upload script
```

## Example: Complete Remote Debugger Feature File

```gherkin
##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2026 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

# Source: src/rrdEventProcess.c
# Source: src/rrdJsonParser.c
# Source: src/rrdRunCmdThread.c
# Source: src/rrdCommandSanity.c

Feature: Remote Debugger Static Report

  Scenario: Check if remote debugger configuration file exists
    Given the configuration file path is set
    When I check if the configuration file exists
    Then the configuration file should exist

  Scenario: Check if /tmp/rrd output directory exists
    Given the /tmp/rrd directory path is set
    When I check if the /tmp/rrd directory exists
    Then the /tmp/rrd directory should exist

  Scenario: Verify remote debugger process is running
    Given the remote debugger process is not running
    When I start the remote debugger process
    Then the remote debugger process should be running

  Scenario: Send WebPA event for IssueType and verify end-to-end processing
    Given the remote debugger is running
    When I trigger the event "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType"
    Then the event for RRD_SET_ISSUE_EVENT should be received
    And the logs should contain "SUCCESS: Message sending Done"
    And the logs should be seen with "SUCCESS: Message Reception Done"
    When the remotedebugger received the message from webPA event
    Then remotedebugger should read the Json file
    And remotedebugger logs should contain the Json File Parse success
    And the issue data node and sub-node should be found in the JSON file
    And the directory should be created to store the executed output
    And Sanity check to validate the commands should be executed
    And Command output should be added to the output file
    And the issuetype systemd service should start successfully
    And the journalctl service should start successfully
    And the process should sleep with timeout
    And the issuetype systemd service should stop successfully
    And the remotedebugger should call script to upload the debug report

  Scenario: Upload remote debugger debug report
    When I check the upload status in the logs
    Then the upload should be successful if upload is success
    Or the upload should fail if upload fails
```

## Integration with Gap Analysis

After generating feature files, use them for gap analysis against the L2 test suite:

### Step 1: Map Features to Existing Tests

```
features/rrd_start_subscribe_and_wait.feature           ↔ tests/test_rrd_start_subscribe_and_wait.py
features/rrd_start_control.feature                      ↔ tests/test_rrd_start_control.py
features/rrd_single_instance.feature                    ↔ tests/test_rrd_single_instance.py
features/rrd_static_profile_report.feature              ↔ tests/test_rrd_static_profile_report.py
features/rrd_static_profile_category_report.feature     ↔ tests/test_rrd_static_profile_category_report.py
features/rrd_background_cmd_static_profile_report.feature ↔ tests/test_rrd_background_cmd_static_profile_report.py
features/rrd_dynamic_profile_report.feature             ↔ tests/test_rrd_dynamic_profile_report.py
features/rrd_dynamic_profile_subcategory_report.feature ↔ tests/test_rrd_dynamic_subcategory_report.py
features/rrd_dynamic_profile_missing_report.feature     ↔ tests/test_rrd_dynamic_profile_missing_report.py
features/rrd_append_report.feature                      ↔ tests/test_rrd_append_report.py
features/rrd_append_dynamic_profile_static_not_found.feature ↔ tests/test_rrd_append_dynamic_profile_static_notfound.py
features/rrd_harmful_command_static_report.feature      ↔ tests/test_rrd_harmful_command_static_report.py
features/test_rrd_dynamic_profile_harmful_report.feature ↔ tests/test_rrd_dynamic_profile_harmful_report.py
features/rrd_corrupted_static_profile_report.feature    ↔ tests/test_rrd_corrupted_static_profile_report.py
features/rrd_static_profile_missing_command_report.feature ↔ tests/test_rrd_static_profile_missing_command_report.py
features/rrd_empty_issuetype_event.feature              ↔ tests/test_rrd_empty_issuetype_event.py
features/rrd_deepsleep_static_report.feature            ↔ tests/test_rrd_deepsleep_static_report.py
features/rrd_debug_report_upload.feature                ↔ tests/test_rrd_debug_report_upload.py
features/rrd_c_api_upload.feature                       ↔ tests/test_rrd_c_api_upload.py
features/test_rrd_static_profile_report_with_suffix.feature ↔ tests/test_rrd_static_profile_report_with_suffix.py
features/test_rrd_static_profile_report_with_suffix_negative_case.feature ↔ tests/test_rrd_static_profile_report_with_suffix_negative_case.py
```

### Step 2: Count Coverage

For each feature file:
1. Count total scenarios (= total testable behaviors)
2. Count scenarios that have a matching `test_*` function in `test/functional-tests/tests/`
3. Calculate coverage = matched / total

### Step 3: Identify Missing Tests

Features without test coverage fall into categories:

| Category | Example | Required Infrastructure |
|---|---|---|
| WebCfg event handling | WebCfgData RBUS event trigger | WebConfig mock, MsgPack payload |
| Profile data SET/GET | setProfileData / getProfileData | `rbuscli` SET/GET validation |
| Upload lock contention | Concurrent upload attempts | Lock file manipulation |
| Deep sleep edge cases | Deep sleep during active collection | IARM event simulation |
| Archive CPU throttle | CPU usage too high during archive | CPU load simulation |
| Configuration fallback | RFC → DCM → dcm.properties chain | Config file manipulation |

### Step 4: Identify Undocumented Tests

Tests that exist in `test/functional-tests/tests/` but have no matching scenario in the
`test/functional-tests/features/` files. These should be documented retroactively:

- `test_rrd_profile_data.py` — Profile data SET/GET (no matching `.feature`)

### Step 5: Generate Gap Report

Include a summary table in `test/functional-tests/features/README.md`:

```markdown
| Behavior Area | Feature Scenarios | L2 Tests | Coverage | Top Gaps |
|---|:---:|:---:|:---:|---|
| Daemon startup/subscribe | 3 | 3 | 100% | — |
| RFC enable/disable | 2 | 2 | 100% | — |
| Single instance | 1 | 1 | 100% | — |
| Static profile report | 5 | 5 | 100% | — |
| Static category report | 5 | 5 | 100% | — |
| Dynamic profile report | 5 | 5 | 100% | — |
| Append mode | 4 | 4 | 100% | — |
| Harmful commands | 4 | 4 | 100% | — |
| Corrupted profile | 4 | 4 | 100% | — |
| Empty issuetype | 2 | 2 | 100% | — |
| Deep sleep | 5 | 5 | 100% | — |
| Debug report upload | 7 | 7 | 100% | — |
| C API upload | 8+ | 8+ | ~100% | Error path coverage |
| WebCfg event | 0 | 0 | 0% | Entire flow |
| Profile data SET/GET | 0 | 1 | partial | No feature file |
| Upload lock contention | 0 | 0 | 0% | Concurrency tests |
```

## Current L2 Test Layout

```
test/functional-tests/
├── features/                                     # BDD feature files (documentation + test specs)
│   ├── rrd_start_subscribe_and_wait.feature      # RBUS subscription
│   ├── rrd_start_control.feature                 # RFC enable/disable
│   ├── rrd_single_instance.feature               # Single instance
│   ├── rrd_static_profile_report.feature         # Static profile end-to-end
│   ├── rrd_static_profile_category_report.feature
│   ├── test_rrd_static_profile_report_with_suffix.feature
│   ├── test_rrd_static_profile_report_with_suffix_negative_case.feature
│   ├── rrd_background_cmd_static_profile_report.feature
│   ├── rrd_dynamic_profile_report.feature
│   ├── rrd_dynamic_profile_subcategory_report.feature
│   ├── rrd_dynamic_profile_missing_report.feature
│   ├── rrd_append_report.feature
│   ├── rrd_append_dynamic_profile_static_not_found.feature
│   ├── rrd_harmful_command_static_report.feature
│   ├── test_rrd_dynamic_profile_harmful_report.feature
│   ├── rrd_corrupted_static_profile_report.feature
│   ├── rrd_static_profile_missing_command_report.feature
│   ├── rrd_empty_issuetype_event.feature
│   ├── rrd_deepsleep_static_report.feature
│   ├── rrd_debug_report_upload.feature
│   └── rrd_c_api_upload.feature
├── tests/                                        # Runnable pytest functions
│   ├── helper_functions.py                       # Log grep, process control, file checks, constants
│   ├── test_rrd_start_subscribe_and_wait.py
│   ├── test_rrd_start_control.py
│   ├── test_rrd_single_instance.py
│   ├── test_rrd_static_profile_report.py
│   ├── test_rrd_static_profile_category_report.py
│   ├── test_rrd_static_profile_report_with_suffix.py
│   ├── test_rrd_static_profile_report_with_suffix_negative_case.py
│   ├── test_rrd_background_cmd_static_profile_report.py
│   ├── test_rrd_dynamic_profile_report.py
│   ├── test_rrd_dynamic_subcategory_report.py
│   ├── test_rrd_dynamic_profile_missing_report.py
│   ├── test_rrd_append_report.py
│   ├── test_rrd_append_dynamic_profile_static_notfound.py
│   ├── test_rrd_harmful_command_static_report.py
│   ├── test_rrd_dynamic_profile_harmful_report.py
│   ├── test_rrd_corrupted_static_profile_report.py
│   ├── test_rrd_static_profile_missing_command_report.py
│   ├── test_rrd_empty_issuetype_event.py
│   ├── test_rrd_deepsleep_static_report.py
│   ├── test_rrd_debug_report_upload.py
│   ├── test_rrd_c_api_upload.py
│   ├── test_rrd_profile_data.py                  # ⚠ No matching .feature file
│   ├── create_json.sh                            # JSON profile creation helper
│   ├── deepsleep_main.c                          # Deep sleep simulation binary
│   ├── power_controller.h                        # Power controller mock header
│   ├── Makefile                                  # Test build file
│   └── uploadSTBLogs.sh                          # Mock upload script
```

**Test runner:** `pytest`, executed sequentially per test file.
**Interfaces exercised:** `rbuscli` (RBUS event trigger), log scraping (`/opt/logs/remotedebugger.log.0`), file system checks (`/tmp/rrd/`, `/etc/rrd/`), mock xconf server (upload validation).

## Maintenance

When remote debugger source code changes:

1. **New RBUS event/parameter added** — Add scenario to appropriate `.feature` file; update RBUS data elements table in README
2. **New profile processing mode** — Create a new `.feature` file; add to README index
3. **New upload mechanism** — Document the upload flow; add C API test scenarios if applicable
4. **Handler removed** — Remove corresponding scenario; note in gap analysis
5. **Build flag changed** — Update conditional compilation notes in README
6. **L2 test added** — Update gap analysis coverage numbers; create `.feature` file if missing
7. **New log messages added** — Update log assertion strings in relevant feature scenarios
8. **New sanity check rules** — Add harmful command detection scenarios
9. **Version tag** — Include generation date in README

## Related Skills

- `technical-documentation-writer` — For detailed architecture and API docs (`docs/`)
- `memory-safety-analyzer` — For safety analysis of C source code
- `thread-safety-analyzer` — For concurrency analysis of event thread and message queue
- `quality-checker` — For running static analysis and build verification
- `triage-logs` — For correlating device logs with remote debugger source code
