---
name: triage-logs
description: >
  Triage remote debugger behavioral issues on RDK devices by correlating
  device log bundles with the remote debugger source tree. Covers daemon
  startup failures, profile processing issues, uploadRRDLogs failures,
  RBUS communication problems, configuration errors, and archive or cleanup
  failures. The user states the issue; this skill guides systematic
  root-cause analysis regardless of issue type.
---

# Remote Debugger Log Triage Skill

## Purpose

Systematically correlate device log bundles with the remote debugger source
code to identify likely root causes, characterize impact, and propose unit
and functional test reproductions for any behavioral anomaly reported by the
user.

---

## Usage

Invoke this skill when:
- A device log bundle is available under `logs/` or is attached separately
- The user reports a remote debugger problem such as startup failure, missing
  report generation, dynamic or static profile issues, upload failure, RBUS
  event issues, or archive cleanup problems
- You need to design a reproduction scenario or propose validation coverage

The user's stated issue drives the investigation. Do not assume a fixed failure
mode.

---

## Step 1: Orient to the Log Bundle

Typical files to inspect first:

```text
logs/<MAC>/<SESSION_TIMESTAMP>/logs/
    remotedebugger.log.0           <- Primary remote debugger daemon log
    messages.txt.0                 <- System messages and crashes
    top_log.txt.0                  <- CPU and memory snapshots
    remote_debugger.json           <- Static profile configuration if captured
    /tmp/rrd/                      <- Generated report and working files
    /opt/logs/                     <- Source logs referenced by uploads
```

Additional files may exist depending on platform integration. If the issue
involves shell orchestration, also look for output associated with
`uploadRRDLogs.sh`.

Note on legacy naming: the remote debugger code still consumes some legacy
configuration artifacts such as `/tmp/DCMSettings.conf` through
`rrd_config_parse_dcm_settings()`. Treat those as compatibility inputs for the
remote debugger rather than evidence of a separate component.

---

## Step 2: Map Startup and Major Components

Read the startup portion of `remotedebugger.log.0` and identify:

| What to find | Log pattern |
|---|---|
| Daemon start | `starting remote-debugger` or `RFC is enabled` |
| Daemon stop | `stopping remote-debugger` or `RFC is disabled` |
| RBUS initialization | `rbus_open`, `Subscribe`, `rrdRbusHandle` |
| Profile load | `remote_debugger.json`, static profile parsing |
| Dynamic event handling | issue type, append, dynamic profile logs |
| Upload orchestration | `uploadRRDLogs`, archive generation, HTTP upload |

Key remote debugger components in this repo:
- Main daemon lifecycle: `src/rrdMain.c`
- RBUS integration and subscriptions: `src/rrdInterface.c`, `src/rrdRbus.h`
- Configuration loading: `src/rrd_config.c`
- Static and dynamic profile handling: `src/rrdJsonParser.c`, `src/rrdDynamic.c`, `src/rrdEventProcess.c`
- Script execution and orchestration: `src/rrdExecuteScript.c`, `src/uploadRRDLogs.c`
- Archive and upload pipeline: `src/rrd_archive.c`, `src/rrd_upload.c`
- Command execution safety: `src/rrdRunCmdThread.c`, `src/rrdCommandSanity.c`

---

## Step 3: Identify the Anomaly Window

Based on the user's stated issue, search for the relevant evidence pattern.

### Remote Debugger Daemon Not Starting or Crashing

```bash
grep -n "remote-debugger\|RFC is enabled\|RFC is disabled\|ERROR\|FATAL\|Segmentation\|core" remotedebugger.log.0
grep -n "remotedebugger\|crash\|oom\|killed" messages.txt.0 | tail -50
```

Check for:
- RBUS open or subscribe failures
- Invalid or missing static profile JSON
- RFC disabled path unexpectedly stopping the daemon
- Dependency or library failures surfaced in system logs

### Static or Dynamic Profile Processing Failures

```bash
grep -n "profile\|dynamic\|static\|issue\|append\|category\|subcategory\|command" remotedebugger.log.0
```

Look for:
- Missing static profile entries in `remote_debugger.json`
- Dynamic profile append failures
- Rejected or harmful commands
- Missing issue type or category mapping

### Upload and Archive Failures

```bash
grep -n "uploadRRDLogs\|archive\|tar\|gzip\|HTTP\|curl\|Failed" remotedebugger.log.0
grep -n "lock\|cleanup\|retry\|upload" remotedebugger.log.0
```

Look for:
- Archive creation failures
- Lock contention in `rrd_upload_check_lock()` or `rrd_upload_wait_for_lock()`
- Upload endpoint, protocol, or HTTP link misconfiguration
- Source directory cleanup failures after upload

### Configuration Failures

```bash
grep -n "config\|properties\|RFC\|DCMSettings\|parse" remotedebugger.log.0
```

Verify:
- `/etc/include.properties` and `/etc/device.properties` availability
- RFC lookup results from `rrd_config_query_rfc()`
- Compatibility parsing from `/tmp/DCMSettings.conf`
- Presence of required fields such as log server, protocol, and upload link

### RBUS Communication Failures

```bash
grep -n "rbus\|subscribe\|unsubscribe\|RRD_\|REMOTE_DEBUGGER_RBUS_HANDLE_NAME" remotedebugger.log.0
```

Verify:
- `rtrouted` availability
- Event subscription success
- Property get or set failures
- Force-sync and issue-event handling paths

### Command Execution and Script Issues

```bash
grep -n "systemd-run\|uploadRRDLogs.sh\|script\|command\|sanity" remotedebugger.log.0
```

Check for:
- Script execution path failures in `rrdExecuteScript.c`
- Background command handling issues
- Command sanitization rejection

---

## Step 4: Correlate with Source Code

Map the observed evidence to source files:

| Issue Area | Source Files |
|---|---|
| Daemon initialization and RFC gating | `src/rrdMain.c` |
| RBUS interface and subscriptions | `src/rrdInterface.c`, `src/rrdRbus.h` |
| Configuration loading | `src/rrd_config.c`, `src/rrd_config.h` |
| Static profile parsing | `src/rrdJsonParser.c` |
| Dynamic profile processing | `src/rrdDynamic.c`, `src/rrdEventProcess.c` |
| Command execution | `src/rrdRunCmdThread.c`, `src/rrdExecuteScript.c`, `src/rrdCommandSanity.c` |
| Upload orchestration | `src/uploadRRDLogs.c`, `src/rrd_upload.c` |
| Archive creation | `src/rrd_archive.c` |
| System information collection | `src/rrd_sysinfo.c` |
| Log processing | `src/rrd_logproc.c` |

Example correlation:

If logs show repeated upload lock waits or cleanup errors:
1. Check `src/rrd_upload.c` for lock handling and cleanup sequencing
2. Check `src/uploadRRDLogs.c` for configuration and archive naming
3. Verify source directory lifecycle around `rrd_upload_cleanup_source_dir()`

---

## Step 5: Reproduce Locally

### Configuration Reproduction

Use the existing unit test binary when possible:

```bash
sh run_ut.sh
```

For targeted config issues, focus on existing tests around `rrd_config_load()`
and upload orchestration in `src/unittest/rrdUnitTestRunner.cpp`, then inspect
`rrd_config_parse_dcm_settings()` directly in `src/rrd_config.c` because there
is not currently a dedicated unit test covering that parser.

### L2 Reproduction

Use the repo's functional test harness:

```bash
sh cov_build.sh
sh run_l2.sh
```

Relevant L2 areas include:
- dynamic profile reports
- static profile reports
- harmful command rejection
- start control and single-instance behavior
- debug report upload and C API upload

### RBUS Reproduction

```bash
systemctl status rtrouted
rbuscli get Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RemoteDebugger.Enable
```

Use the actual property names observed in logs or in `src/rrdRbus.h` when a
platform variant differs.

---

## Step 6: Test Gap Analysis

Identify code paths that may lack regression coverage.

### Unit Test Coverage

Inspect `src/unittest/rrdUnitTestRunner.cpp` for:
- error-path coverage in `rrd_config_load()`
- upload orchestration failures
- archive cleanup behavior
- RBUS failure handling
- harmful command rejection

### L2 Test Coverage

Inspect `test/functional-tests/features/` and `test/functional-tests/tests/` for:
- missing scenarios matching the reported bug
- race or timing-sensitive startup cases
- malformed static profile inputs
- upload failure and retry edge cases

---

## Step 7: Propose Fix and Test

### Fix Template

```c
// BEFORE: Continue after upload failure
int ret = rrd_upload_execute(server, protocol, link, workdir, archive, source_dir);
// Continued processing here

// AFTER: Stop and surface the failure
int ret = rrd_upload_execute(server, protocol, link, workdir, archive, source_dir);
if (ret != 0) {
    RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "Upload failed: %d\n", ret);
    return ret;
}
```

### Test Template

```cpp
TEST(RemoteDebuggerUploadTest, RejectsUploadFailureAndStopsCleanupSequence)
{
    // Configure mocks to force upload failure, then verify return path.
    int ret = rrd_upload_orchestrate("/tmp/rrd-test", "issue_type");
    EXPECT_NE(ret, 0);
}
```

---

## Output Format

Present findings in this structure:

```markdown
## Triage Summary

**Issue:** <user's stated problem>
**Evidence:** <key log excerpts with timestamps>
**Root Cause:** <likely cause based on code analysis>
**Impact:** <what fails, user-visible effect>

## Code Location

**File:** <source file path>
**Function:** <function name>
**Line:** <approximate line number>

## Reproduction

[bash or test scenario to reproduce]

## Proposed Fix

[code diff or description]

## Test Coverage

**Existing:** [what tests exist]
**Missing:** [tests needed to prevent regression]

## Next Steps

1. [immediate action]
2. [follow-up verification]
```

---

## Example Triage Flow

**User:** "remote debugger generates the archive but upload never completes"

**Step 1:** Located `remotedebugger.log.0`, found repeated lock wait and upload failures.

**Step 2:** Checked `src/rrd_upload.c` and confirmed the failure path depends on
lock state plus upload return code.

**Step 3:** Verified configuration inputs in `rrd_config_load()` and confirmed
the upload server or link was missing from the effective configuration.

**Root Cause:** Effective upload configuration was incomplete, causing the
upload path to fail after archive preparation.

**Fix Direction:** Add stronger validation before calling
`rrd_upload_execute()` and extend unit coverage for incomplete configuration.
