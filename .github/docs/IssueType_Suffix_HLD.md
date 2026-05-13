# RRD UUID Support — High-Level Design Document

## Document Information

| Field             | Value                                   |
|-------------------|-----------------------------------------|
| **Feature**       | Optional Underscore-Delimited Suffix on IssueType |
| **Component**     | Remote Debugger (RRD)                   |
| **Version**       | 1.0                                     |
| **Date**          | May 2026                                |
| **Target Platform** | Embedded Linux Systems               |

---

## 1. Executive Summary

This document describes the high-level design for adding support for an **optional, underscore-delimited suffix** appended to the `IssueType` RFC value (e.g., `Device.DeviceInfo_Search-1234`). The suffix is parsed, validated, sanitized, and carried through the entire event-processing pipeline so that it can be re-appended to the generated upload archive filename. Additionally, systemd unit names are disambiguated using an epoch timestamp, and the debug output file open-mode is corrected to prevent stale data accumulation.

---

## 2. Background and Motivation

### 2.1 Problem Statement

Previously, the `IssueType` RFC parameter was expected to contain a simple, period-delimited string (e.g., `Device.DeviceInfo`). Operators needed a way to attach additional context (e.g., a ticket number or short identifier) directly to the event value so that the resulting debug archive could be uniquely correlated with an external issue tracker without modifying the core node/sub-node classification logic.

### 2.2 Goals

1. Parse and validate an optional `_<suffix>` token appended to the IssueType string.
2. Sanitize the suffix to prevent shell injection when it is embedded in filenames or commands.
3. Propagate the suffix through the event pipeline to the archive upload step.
4. Append the suffix to the upload archive filename so the remote server can correlate the archive with the originating issue context.
5. Disambiguate systemd transient unit names to avoid conflicts when the same IssueType is processed concurrently or repeatedly.

### 2.3 Non-Goals

- Changing the node/sub-node classification logic.
- Supporting suffixes longer than 9 characters (including the leading `_`).
- Modifying the upstream RFC parameter interface.

---

## 3. Architecture Overview

### 3.1 System Context

```mermaid
graph TB
    subgraph Device["Embedded Device"]
        RFC["RFC/RBUS Event<br/>IssueType = Device.Info_ab1234"]
        subgraph RRD["Remote Debugger (RRD) Process"]
            Interface["rrdInterface<br/>Event Reception &amp; Routing"]
            EventProc["rrdEventProcess<br/>IssueType Processing"]
            JsonParser["rrdJsonParser<br/>split_issue_type() + JSON Lookup"]
            CmdThread["rrdRunCmdThread<br/>Command Execution"]
            LogProc["rrd_logproc<br/>Log Conversion"]
            Archive["rrd_archive<br/>Tar/Gz Creation"]
            Upload["rrd_upload<br/>S3/HTTP Upload"]
        end
        Interface -->|"data_buf (mdata, suffix)"| EventProc
        EventProc -->|"data_buf"| JsonParser
        JsonParser -->|"issueData + suffix"| CmdThread
        CmdThread -->|"debug_outputs.txt"| Archive
        Archive -->|"<IssueType><suffix>.tar.gz"| Upload
    end
    Upload -->|"HTTPS"| RemoteServer["Remote Log Server"]
```

### 3.2 Module Responsibilities

| Module | File(s) | Responsibility |
|--------|---------|----------------|
| Event Interface | `rrdInterface.c` | Receives RBUS/RFC events; initialises and de-allocates `data_buf`; propagates `suffix = NULL` initially |
| Event Processor | `rrdEventProcess.c` | Splits IssueType list; calls `split_issue_type()`; populates `data_buf.suffix`; routes to JSON parser |
| JSON Parser | `rrdJsonParser.c` | Implements `split_issue_type()`; resolves node/sub-node; builds upload filename with suffix |
| Command Thread | `rrdRunCmdThread.c` | Executes debug commands via systemd-run; writes `debug_outputs.txt` (w+ mode); appends epoch to unit name |
| Log Processor | `rrd_logproc.c` | Converts IssueType to safe filename characters; preserves hyphens for suffix UUID tokens |
| Archive Manager | `rrd_archive.c` | Creates `.tar.gz` archive of the output directory |
| Upload Manager | `rrd_upload.c` | Uploads the archive to the remote log server |
| Common Types | `rrdCommon.h` | Defines `data_buf` struct with new `suffix` field |

---

## 4. Data Flow

### 4.1 Nominal Flow (with suffix)

```
RFC set: Device.DeviceInfo_ab1234
       |
       v
rrdInterface._remoteDebuggerEventHandler()
  → RRD_data_buff_init()  [suffix = NULL]
  → pushIssueTypesToMsgQueue("Device.DeviceInfo_ab1234", EVENT_MSG)
       |
       v
rrdEventProcess.processIssueTypeEvent()
  → issueTypeSplitter("Device.DeviceInfo_ab1234", ',', &cmdMap)
  → for each token: split_issue_type(token, base, suffix)
      base   = "Device.DeviceInfo"
      suffix = "_ab1234"
  → cmdBuff->mdata  = "Device.DeviceInfo"
  → cmdBuff->suffix = "_ab1234"
  → processIssueType(cmdBuff)
       |
       v
rrdJsonParser.checkIssueNodeInfo()
  → Reads JSON profile for "Device.DeviceInfo"
  → outdir = "/tmp/rrd/DeviceInfo-DebugReport-2026-05-13-10-00-00"
  → executeCommands() / invokeSanityandCommandExec()
       |
       v
rrdRunCmdThread.executeCommands()
  → unit = "remote_debugger_DeviceInfo_<epoch>"
  → systemd-run --unit=remote_debugger_DeviceInfo_<epoch> ...
  → journalctl -u remote_debugger_DeviceInfo_<epoch>
  → fopen(debug_outputs.txt, "w+")
       |
       v
rrdJsonParser.checkIssueNodeInfo()  [after exec]
  → tarName = "Device.DeviceInfo" + "_ab1234"
            = "Device.DeviceInfo_ab1234"
  → uploadDebugoutput(outdir, "Device.DeviceInfo_ab1234")
       |
       v
rrd_logproc.rrd_logproc_convert_issue_type("Device.DeviceInfo_ab1234")
  → "DEVICE_DEVICEINFO_AB1234"   (hyphens also preserved)
       |
       v
rrd_upload / rrd_archive
  → Archive: AABBCCDDEEFF_DEVICE_DEVICEINFO_AB1234.tar.gz
  → Upload to remote server
```

### 4.2 Nominal Flow (without suffix)

Same as above with:
- `split_issue_type()` returns `suffix = ""`
- `tarName = "Device.DeviceInfo"` (no suffix appended)
- Archive: `AABBCCDDEEFF_DEVICE_DEVICEINFO.tar.gz`

---

## 5. Key Algorithms and Data Structures

### 5.1 `data_buf` Structure (updated)

```c
typedef struct mbuffer {
    message_type_et     mtype;
    char               *mdata;      /* IssueType base (no suffix) */
    char               *jsonPath;
    bool                inDynamic;
    bool                appendMode;
    deepsleep_event_et  dsEvent;
    char               *suffix;     /* NEW: optional "_<token>", heap-allocated or NULL */
} data_buf;
```

**Lifecycle of `suffix`:**
- Initialised to `NULL` by `RRD_data_buff_init()`.
- Allocated (via `strdup`) inside `processIssueTypeEvent()` when a non-empty suffix is present.
- Freed in all exit paths of `checkIssueNodeInfo()` and `processIssueTypeInInstalledPackage()`.
- Also freed by `RRD_data_buff_deAlloc()` for callers using the generic de-allocation path.

### 5.2 `split_issue_type()` Algorithm

```
INPUT:  input     – raw IssueType string (e.g. "Device.DeviceInfo_ab1234")
        base_len  – size of base buffer
        suffix_len – size of suffix buffer
OUTPUT: base   – part before first '_' (or full input if no '_')
        suffix – part from first '_' onwards IF len ≤ RRD_MAX_SUFFIX_LEN (9),
                 sanitized to [A-Za-z0-9_-]; otherwise ""

ALGORITHM:
1.  Initialise base[0] = '\0', suffix[0] = '\0'.
2.  Validate pointers and buffer sizes; return on failure.
3.  Find first '_' in input using strchr().
4.  If found:
    a. Copy characters before '_' into base (capped at base_len-1).
    b. If strlen(underscore_ptr) ≤ RRD_MAX_SUFFIX_LEN:
         Iterate over underscore_ptr:
           Accept char if isalnum() || '_' || '-'
           Else discard (injection prevention)
         Null-terminate suffix.
    c. Else:
         Log debug message; suffix = "".
5.  If not found:
    Copy full input into base (capped at base_len-1); suffix = "".
```

**Constant:**
```c
#define RRD_MAX_SUFFIX_LEN  9   /* maximum strlen() of the suffix, measured from the
                                   leading '_' inclusive — e.g. "_ab12345" has strlen 8 */
```

The length check is `strlen(underscore_ptr) <= RRD_MAX_SUFFIX_LEN`, where `underscore_ptr` points at the `_` character. Therefore the `_` itself is part of the measured length and the token portion may be at most 8 characters long (e.g. `_ab12345` → strlen 8 ≤ 9 → accepted).

**Examples:**

| Input | Base | Suffix | `strlen` from `_` | Decision |
|-------|------|--------|-------------------|----------|
| `Device.DeviceInfo_ab1234` | `Device.DeviceInfo` | `_ab1234` | 7 ≤ 9 | accepted |
| `Device.DeviceInfo_ab12345` | `Device.DeviceInfo` | `_ab12345` | 8 ≤ 9 | accepted |
| `Device.DeviceInfo_ab;rm` | `Device.DeviceInfo` | `_abrm` | 5 ≤ 9; `;` stripped | accepted (sanitised) |
| `Device.DeviceInfo_Search-uuid-long` | `Device.DeviceInfo` | `` | 21 > 9 | discarded |
| `Device.DeviceInfo` | `Device.DeviceInfo` | `` | no `_` present | no suffix |

### 5.3 Systemd Unit Name Disambiguation

```c
time_t epochTime = time(NULL);
/* Note: time() returns (time_t)-1 on error; snprintf will render it as "-1",
   which is still a valid unique string and causes no functional regression —
   the unit name will be unusual but won't conflict with other invocations. */
snprintf(remoteDebuggerServiceStr, sizeof(remoteDebuggerServiceStr),
         "%s%s%ld", remoteDebuggerPrefix, cmdData->rfcvalue, (long)epochTime);
/* Result: "remote_debugger_DeviceInfo_1715600000" */
```

Previously the unit name was static per `rfcvalue`, causing `systemctl start` collisions when the same IssueType was processed in rapid succession. Appending the epoch timestamp guarantees uniqueness per invocation.

### 5.4 `debug_outputs.txt` Open Mode Change

| Before | After | Reason |
|--------|-------|--------|
| `fopen(finalOutFile, "a+")` | `fopen(finalOutFile, "w+")` | Truncate-on-open prevents stale output from a previous run contaminating the current report |

### 5.5 Hyphen Preservation in `rrd_logproc_convert_issue_type()`

```c
/* Before: hyphens were dropped (only alnum and '_'/'.' were mapped) */
/* After:  hyphens are preserved so suffix UUID tokens remain distinct */
else if (c == '-') output[j++] = '-';
```

---

## 6. Component Interaction and Interfaces

### 6.1 Public API Changes

| Symbol | File | Change |
|--------|------|--------|
| `split_issue_type()` | `rrdJsonParser.h / .c` | **New** function |
| `data_buf.suffix` | `rrdCommon.h` | **New** field (`char *`) |
| `RRD_data_buff_init()` | `rrdInterface.c` | Sets `sbuf->suffix = NULL` |
| `RRD_data_buff_deAlloc()` | `rrdInterface.c` | `free(sbuf->suffix)` added |

### 6.2 Internal Call Chain

```
processIssueTypeEvent()
  └─ split_issue_type()              [rrdJsonParser.c]
  └─ processIssueType()              [rrdEventProcess.c]
       └─ processIssueTypeInStaticProfile()
            └─ checkIssueNodeInfo()  [rrdJsonParser.c]
                 └─ executeCommands() / invokeSanityandCommandExec()
                 └─ uploadDebugoutput(outdir, tarName)
                      └─ rrd_logproc_convert_issue_type()
                      └─ rrd_archive_create()
                      └─ rrd_upload_file()
```

---

## 7. Flowcharts

### 7.1 `split_issue_type()` Flowchart

```mermaid
flowchart TD
    A([Start]) --> B{Pointers valid &\nbuffer sizes > 0?}
    B -- No --> Z([Return: base='', suffix=''])
    B -- Yes --> C{First '_'\nfound in input?}
    C -- No --> D[Copy full input to base]
    D --> Z2([Return: suffix=''])
    C -- Yes --> E[Copy chars before '_' to base]
    E --> F{strlen from '_'\n<= 9?}
    F -- No --> G[Log debug: suffix discarded]
    G --> Z2
    F -- Yes --> H[Iterate suffix chars]
    H --> I{char isalnum\nor '_' or '-'?}
    I -- Yes --> J[Copy char to suffix]
    I -- No --> K[Skip char]
    J --> L{More chars?}
    K --> L
    L -- Yes --> H
    L -- No --> M[Null-terminate suffix]
    M --> Z3([Return: base, suffix])
```

**Text-based alternative:**
```
START
  │
  ├─[NULL / zero-size checks fail]──→ RETURN (base="", suffix="")
  │
  ├─[no '_' in input]──→ base = full input, suffix="" → RETURN
  │
  ├─[underscore found]
  │    │
  │    ├─ base = input[0..underscore)
  │    │
  │    ├─[len(underscore_part) > 9]──→ log + suffix="" → RETURN
  │    │
  │    └─[len(underscore_part) <= 9]──→ sanitize → suffix → RETURN
END
```

### 7.2 IssueType Event Processing with Suffix

```mermaid
flowchart TD
    A([RBUS Event]) --> B[rrdInterface:\nReceive IssueType RFC value]
    B --> C[RRD_data_buff_init:\nsuffix = NULL]
    C --> D[pushIssueTypesToMsgQueue]
    D --> E[processIssueTypeEvent:\nissueTypeSplitter - split by comma]
    E --> F{For each\nIssueType token}
    F --> G[split_issue_type:\nbase + suffix]
    G --> H{base\nempty?}
    H -- Yes --> I[Skip token, free resources]
    I --> F
    H -- No --> J[Allocate data_buf:\nmdata = base\nsuffix = strdup of suffix]
    J --> K[processIssueType]
    K --> L[JSON profile lookup\nfor base node]
    L --> M[checkIssueNodeInfo:\ncreate outdir\nexecute commands]
    M --> N{exec\nsuccess?}
    N -- No --> O[Skip upload\nfree suffix]
    N -- Yes --> P{suffix\nnon-empty?}
    P -- Yes --> Q[tarName = base + suffix]
    P -- No --> R[tarName = base]
    Q --> S[uploadDebugoutput\ntarName]
    R --> S
    S --> T[free suffix\nfree mdata\nfree buff]
    T --> F
    F -->|all done| U([End])
```

### 7.3 Systemd Unit Name Disambiguation

```mermaid
flowchart TD
    A[executeCommands called] --> B[Get current epoch: time NULL]
    B --> C[Build unit name:\nremote_debugger_ + rfcvalue + epoch]
    C --> D[systemd-run --unit=<name>\n--service-type=oneshot]
    D --> E[journalctl -u <name>]
    E --> F[fopen debug_outputs.txt w+]
    F --> G[write command header]
    G --> H[copy systemd stdout to file]
    H --> I[copy journalctl output to file]
    I --> J[fclose]
    J --> K[sleep timeout]
    K --> L[systemctl stop <name>]
    L --> M([Return])
```

---

## 8. Sequence Diagrams

### 8.1 IssueType with Suffix — End-to-End

```mermaid
sequenceDiagram
    participant RFC as RFC/RBUS
    participant Iface as rrdInterface
    participant EP as rrdEventProcess
    participant JP as rrdJsonParser
    participant CT as rrdRunCmdThread
    participant UL as rrd_upload

    RFC->>Iface: set IssueType = "Device.Info_ab1234"
    Iface->>Iface: RRD_data_buff_init(suffix=NULL)
    Iface->>EP: pushIssueTypesToMsgQueue(EVENT_MSG)
    EP->>JP: split_issue_type("Device.Info_ab1234")
    JP-->>EP: base="Device.Info", suffix="_ab1234"
    EP->>EP: cmdBuff->mdata = "Device.Info"
    EP->>EP: cmdBuff->suffix = strdup("_ab1234")
    EP->>JP: processIssueType(cmdBuff)
    JP->>JP: getIssueInfo / findIssueInParsedJSON
    JP->>CT: executeCommands(issueData)
    CT->>CT: systemd-run --unit=remote_debugger_Info_<epoch>
    CT->>CT: journalctl -u remote_debugger_Info_<epoch>
    CT->>CT: fopen(debug_outputs.txt, "w+")
    CT-->>JP: true (success)
    JP->>JP: tarName = "Device.Info" + "_ab1234"
    JP->>UL: uploadDebugoutput(outdir, "Device.Info_ab1234")
    UL-->>JP: 0 (success)
    JP->>JP: free(buff->suffix), free(buff->mdata)
```

**Text-based alternative:**
```
RFC             rrdInterface    rrdEventProcess  rrdJsonParser   rrdRunCmdThread  rrd_upload
 |                   |                 |               |                |               |
 |--set IssueType--->|                 |               |                |               |
 |                   |--init(buf)----->|               |                |               |
 |                   |--pushToMsgQ---->|               |                |               |
 |                                     |--split()----->|                |               |
 |                                     |<--base,suffix-|                |               |
 |                                     |--processIssue(cmdBuff)-------->|               |
 |                                                     |--execCmds()-->|                |
 |                                                     |<----success---|                |
 |                                                     |--tarName=base+suffix           |
 |                                                     |--uploadDebugoutput()---------->|
 |                                                     |<---------------success---------|
```

### 8.2 Suffix Validation and Rejection

```mermaid
sequenceDiagram
    participant EP as rrdEventProcess
    participant JP as rrdJsonParser (split_issue_type)

    EP->>JP: split_issue_type("Device.Info_Search-uuid-very-long-string", ...)
    JP->>JP: strlen("_Search-uuid-very-long-string") = 29 > 9
    JP->>JP: log: "Suffix exceeds max length; discarding"
    JP-->>EP: base="Device.Info", suffix=""
    EP->>EP: cmdBuff->suffix = NULL (no suffix stored)
    Note over EP,JP: Processing continues with base only
```

---

## 9. Error Handling

| Scenario | Handling |
|----------|----------|
| Suffix length > 9 chars | Logged at DEBUG level; suffix discarded; processing continues with base only |
| Suffix contains unsafe characters (`; & | >` etc.) | Characters stripped during sanitisation; only `[A-Za-z0-9_-]` retained |
| `strdup(suffix)` fails (OOM) | Logged at ERROR; `cmdBuff->suffix` remains `NULL`; processing continues without suffix |
| Empty base after `split_issue_type` | Token skipped; `cmdBuff` freed; iteration continues |
| `snprintf` overflow for `tarName` | Logged at ERROR; upload skipped; suffix freed normally |
| `mkdir` or `chdir` failure | Logged at ERROR; `mdata`, `jsonPath`, `suffix` all freed before return |
| `fopen(debug_outputs.txt)` failure | Logged at ERROR; function returns `false`; no archive created |

---

## 10. Security Considerations

| Concern | Mitigation |
|---------|-----------|
| Shell injection via suffix | Suffix sanitised to `[A-Za-z0-9_-]` by `split_issue_type()` before any use in filenames or command strings |
| Suffix used in systemd unit name | Epoch timestamp is appended to the `rfcvalue` (the base, never the raw suffix), keeping unit name well-controlled |
| Buffer overflow in `tarName` | `snprintf` result checked; error logged and upload skipped on overflow |
| Directory traversal | Suffix never used in directory creation; only in the upload archive filename |

---


## 11. Glossary

| Term | Definition |
|------|-----------|
| IssueType | RFC parameter string identifying the debug category (e.g., `Device.DeviceInfo`) |
| Suffix | Optional `_<token>` appended to IssueType (e.g., `_ab1234`), used to distinguish archive names |
| base | The IssueType string before the first `_`; used for JSON profile lookup |
| RFC | Remote Feature Control — device configuration parameter infrastructure |
| RBUS | RDK Message Bus — event delivery mechanism on RDK devices |
| RRD | RDK Remote Debugger — this component |
| tarName | The logical name used when creating and uploading the debug archive |
| data_buf | Internal message buffer passed between RRD processing stages |
