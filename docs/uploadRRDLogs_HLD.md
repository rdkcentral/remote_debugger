# uploadRRDLogs - High-Level Design Document

## Document Information
- **Component Name:** uploadRRDLogs (C Implementation)
- **Original Script:** uploadRRDLogs.sh
- **Version:** 1.0
- **Date:** December 1, 2025
- **Target Platform:** Embedded Linux Systems

## 1. Executive Summary

This document describes the high-level design for migrating the `uploadRRDLogs.sh` shell script to a C-based implementation. The component is responsible for collecting, archiving, and uploading Remote Debugger (RRD) diagnostic logs to a remote server for analysis. The C implementation will provide improved performance, reduced memory footprint, and better integration with the embedded system environment while maintaining full functional compatibility with the original script.

## 2. Architecture Overview

### 2.1 System Context

```
┌─────────────────────────────────────────────────────────────────┐
│                    Embedded Device System                        │
│                                                                  │
│  ┌────────────────┐                                             │
│  │  RRD Service   │                                             │
│  │  (Trigger)     │                                             │
│  └────────┬───────┘                                             │
│           │                                                      │
│           │ Executes with                                       │
│           │ (uploaddir, issuetype)                             │
│           ▼                                                      │
│  ┌─────────────────────────────────────────────────────┐       │
│  │        uploadRRDLogs (C Program)                     │       │
│  │  ┌──────────────────────────────────────────────┐   │       │
│  │  │  Main Orchestration Layer                    │   │       │
│  │  └─────┬──────────────────────────────────┬─────┘   │       │
│  │        │                                  │          │       │
│  │        ▼                                  ▼          │       │
│  │  ┌──────────────────┐        ┌──────────────────┐   │       │
│  │  │ Configuration    │        │  Log Processing  │   │       │
│  │  │ Manager          │        │  Engine          │   │       │
│  │  └──────────────────┘        └──────────────────┘   │       │
│  │        │                                  │          │       │
│  │        ▼                                  ▼          │       │
│  │  ┌──────────────────┐        ┌──────────────────┐   │       │
│  │  │ System Info      │        │  Archive Manager │   │       │
│  │  │ Provider         │        │                  │   │       │
│  │  └──────────────────┘        └──────────────────┘   │       │
│  │                                       │              │       │
│  │                                       ▼              │       │
│  │                            ┌──────────────────┐     │       │
│  │                            │ Upload Manager   │     │       │
│  │                            └──────────────────┘     │       │
│  └─────────────────────────────────┬────────────────────┘      │
│                                    │                            │
│  ┌─────────────────────────────────┼──────────────┐            │
│  │  External Dependencies          │              │            │
│  │                                 │              │            │
│  │  ┌──────────────┐     ┌─────────▼─────────┐   │            │
│  │  │ tr181 Tool   │     │ uploadSTBLogs.sh  │   │            │
│  │  │ (RFC Params) │     │ (Upload Script)   │   │            │
│  │  └──────────────┘     └───────────────────┘   │            │
│  │                                                 │            │
│  └─────────────────────────────────────────────────┘           │
│                                                                  │
│  ┌─────────────────────────────────────────────────┐           │
│  │  File System                                     │           │
│  │  - /etc/include.properties                      │           │
│  │  - /etc/device.properties                       │           │
│  │  - /tmp/DCMSettings.conf                        │           │
│  │  - /tmp/rrd/ (working directory)                │           │
│  │  - $LOG_PATH/remote-debugger.log                │           │
│  └─────────────────────────────────────────────────┘           │
└──────────────────────────────────────────────────────────────────┘
                                │
                                │ HTTPS/HTTP
                                ▼
                   ┌────────────────────────┐
                   │  Remote Log Server     │
                   │  (DCM/SSR Server)      │
                   └────────────────────────┘
```

### 2.2 High-Level Component View

The application is structured into six major modules:

1. **Main Orchestration Layer:** Entry point and workflow coordination
2. **Configuration Manager:** Configuration loading and parsing
3. **System Info Provider:** System information gathering (MAC, timestamp)
4. **Log Processing Engine:** Directory validation and special handling
5. **Archive Manager:** Log compression and tar archive creation
6. **Upload Manager:** Upload coordination and concurrency control

### 2.3 Design Principles

- **Modularity:** Each functional area encapsulated in separate modules
- **Low Memory Footprint:** Stack allocation preferred over heap
- **Error Resilience:** Comprehensive error handling at all layers
- **Portability:** POSIX-compliant APIs, cross-platform compatible
- **Performance:** Efficient file operations and minimal overhead
- **Maintainability:** Clear interfaces and well-documented code

## 3. Module Breakdown

### 3.1 Main Orchestration Layer

**Module Name:** `rrd_upload_main`

**Purpose:** Program entry point and overall workflow coordination

**Responsibilities:**
- Parse and validate command-line arguments
- Initialize all subsystems
- Coordinate execution flow between modules
- Handle top-level error conditions
- Ensure proper cleanup and resource release
- Return appropriate exit codes

**Key Functions:**
```c
int main(int argc, char *argv[]);
int rrd_upload_orchestrate(const char *upload_dir, const char *issue_type);
void rrd_upload_cleanup(void);
```

**Workflow:**
1. Parse command-line arguments (UPLOADDIR, ISSUETYPE)
2. Initialize logging subsystem
3. Load configuration via Configuration Manager
4. Gather system information
5. Validate and prepare log directory
6. Create archive via Archive Manager
7. Upload via Upload Manager
8. Clean up resources
9. Return status code

**Error Handling:**
- Invalid arguments → Exit with code 1
- Configuration errors → Exit with appropriate code
- Upload failures → Clean up and propagate error code

**Dependencies:**
- All other modules
- Standard C library (stdio, stdlib, string)

---

### 3.2 Configuration Manager

**Module Name:** `rrd_config`

**Purpose:** Load and manage configuration from multiple sources

**Responsibilities:**
- Parse property files (key=value format)
- Query TR-181 parameters via tr181 tool
- Handle configuration priority and fallback
- Provide configuration values to other modules
- Manage configuration data structures

**Key Data Structures:**
```c
typedef struct {
    char log_server[256];
    char http_upload_link[512];
    char upload_protocol[16];
    char rdk_path[256];
    char log_path[256];
    char build_type[32];
    bool use_rfc_config;
} rrd_config_t;
```

**Key Functions:**
```c
int rrd_config_load(rrd_config_t *config);
int rrd_config_parse_properties(const char *filepath, rrd_config_t *config);
int rrd_config_query_rfc(rrd_config_t *config);
int rrd_config_parse_dcm_settings(const char *filepath, rrd_config_t *config);
const char* rrd_config_get_value(const rrd_config_t *config, const char *key);
void rrd_config_cleanup(rrd_config_t *config);
```

**Configuration Priority:**
1. TR-181 RFC parameters (if available and not prod build with /opt/dcm.properties)
2. DCMSettings.conf (/tmp/DCMSettings.conf)
3. dcm.properties (/opt/dcm.properties or /etc/dcm.properties)

**Property File Format:**
```
KEY=value
KEY="value with spaces"
# Comments
```

**RFC Parameters to Query:**
- `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.LogServerUrl`
- `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.SsrUrl`

**Error Handling:**
- Missing files → Try fallback sources
- Parse errors → Log warning, use defaults
- tr181 not available → Skip RFC query
- Empty critical values → Return error

**Dependencies:**
- System Info Provider (for file access)
- File I/O functions
- Process execution (for tr181)

---

### 3.3 System Info Provider

**Module Name:** `rrd_sysinfo`

**Purpose:** Gather system identification and status information

**Responsibilities:**
- Retrieve device MAC address
- Generate formatted timestamps
- Provide system status information
- File and directory validation
- Process and file existence checks

**Key Functions:**
```c
int rrd_sysinfo_get_mac_address(char *mac_addr, size_t size);
int rrd_sysinfo_get_timestamp(char *timestamp, size_t size);
bool rrd_sysinfo_file_exists(const char *filepath);
bool rrd_sysinfo_dir_exists(const char *dirpath);
bool rrd_sysinfo_dir_is_empty(const char *dirpath);
int rrd_sysinfo_get_dir_size(const char *dirpath, size_t *size);
```

**MAC Address Retrieval:**
- Method 1: Parse `/sys/class/net/[interface]/address`
- Method 2: ioctl SIOCGIFHWADDR (fallback)
- Method 3: Execute `getMacAddressOnly` from utils.sh (compatibility)
- Format: XX:XX:XX:XX:XX:XX (colon-separated)

**Timestamp Format:**
- Pattern: `YYYY-MM-DD-HH-MM-SS[AM|PM]`
- Example: `2025-12-01-03-45-30PM`
- Use: `strftime()` with custom formatting

**File System Utilities:**
- Check file/directory existence using `access()` or `stat()`
- Validate permissions
- Check directory contents
- Get file/directory sizes

**Error Handling:**
- MAC address unavailable → Return error
- Timestamp generation failure → Return error
- File system access errors → Return appropriate error codes

**Dependencies:**
- POSIX system calls (stat, access, opendir)
- Network interface access (sys/ioctl.h)
- Time functions (time.h)

---

### 3.4 Log Processing Engine

**Module Name:** `rrd_logproc`

**Purpose:** Validate and prepare log directories for archiving

**Responsibilities:**
- Validate source directory exists and contains files
- Handle special issue type logic (LOGUPLOAD_ENABLE)
- Convert issue type to uppercase
- Prepare working directory
- Move live logs if needed

**Key Functions:**
```c
int rrd_logproc_validate_source(const char *source_dir);
int rrd_logproc_prepare_logs(const char *source_dir, const char *issue_type);
int rrd_logproc_convert_issue_type(const char *input, char *output, size_t size);
int rrd_logproc_handle_live_logs(const char *source_dir);
```

**Validation Steps:**
1. Check directory exists
2. Check directory is readable
3. Check directory contains files (not empty)
4. Verify sufficient space in /tmp

**Issue Type Processing:**
- Convert to uppercase using `toupper()`
- Validate for filesystem safety (no special chars)
- Sanitize if necessary

**Special Handling - LOGUPLOAD_ENABLE:**
1. Check if issue type equals "LOGUPLOAD_ENABLE"
2. Look for `RRD_LIVE_LOGS.tar.gz` in `/tmp/rrd/`
3. Move file to source directory
4. Log operation (success or failure)
5. Continue even if file not found

**Error Handling:**
- Directory not found → Log and return error
- Empty directory → Log and return error
- Insufficient space → Log and return error
- File move failure → Log warning but continue

**Dependencies:**
- System Info Provider (directory checks)
- File system operations

---

### 3.5 Archive Manager

**Module Name:** `rrd_archive`

**Purpose:** Create compressed tar archive of log files

**Responsibilities:**
- Generate archive filename
- Create gzip-compressed tar archive
- Handle large file sets efficiently
- Monitor disk space during creation
- Clean up on errors

**Key Functions:**
```c
int rrd_archive_create(const char *source_dir, 
                       const char *working_dir,
                       const char *archive_filename);
int rrd_archive_generate_filename(const char *mac, 
                                  const char *issue_type,
                                  const char *timestamp,
                                  char *filename,
                                  size_t size);
int rrd_archive_cleanup(const char *archive_path);
```

**Archive Naming Convention:**
```
{MAC}_{ISSUETYPE}_{TIMESTAMP}_RRD_DEBUG_LOGS.tgz
Example: 11:22:33:44:55:66_CRASH_REPORT_2025-12-01-03-45-30PM_RRD_DEBUG_LOGS.tgz
```

**Archive Creation Approach:**

**Option 1: Use libarchive (Preferred)**
- Library: libarchive
- Advantages: Native C API, efficient, portable
- Implementation:
  ```c
  struct archive *a = archive_write_new();
  archive_write_add_filter_gzip(a);
  archive_write_set_format_ustar(a);
  archive_write_open_filename(a, filename);
  // Add files iteratively
  archive_write_close(a);
  ```

**Option 2: Execute tar Command (Fallback)**
- Use `system()` or `fork()/exec()`
- Command: `tar -zcf archive.tgz -C source_dir .`
- Redirect stderr to log file
- Check return code

**Implementation Considerations:**
- Stream processing to minimize memory usage
- No loading entire directory into memory
- Progress monitoring (optional)
- Atomic creation (temp file + rename)

**Error Handling:**
- Disk space exhaustion → Return error, cleanup partial files
- Permission errors → Log and return error
- Source file read errors → Log warning, continue with remaining files
- Archive write errors → Cleanup and return error

**Dependencies:**
- libarchive (if using Option 1)
- System calls (if using Option 2)
- File system operations
- System Info Provider

---

### 3.6 Upload Manager

**Module Name:** `rrd_upload`

**Purpose:** Coordinate log upload with concurrency control

**Responsibilities:**
- Check for concurrent upload operations (lock file)
- Retry logic for lock acquisition
- Execute uploadSTBLogs.sh script
- Monitor upload progress
- Clean up after upload (success or failure)

**Key Functions:**
```c
int rrd_upload_execute(const char *log_server,
                       const char *protocol,
                       const char *http_link,
                       const char *working_dir,
                       const char *archive_filename);
int rrd_upload_check_lock(bool *is_locked);
int rrd_upload_wait_for_lock(int max_attempts, int wait_seconds);
int rrd_upload_invoke_stb_upload(const char *log_server,
                                 const char *protocol,
                                 const char *http_link,
                                 const char *archive_filename);
int rrd_upload_cleanup_files(const char *archive_path, const char *source_dir);
```

**Concurrency Control:**
- **Lock File:** `/tmp/.log-upload.pid`
- **Check Mechanism:** Test file existence with `access()`
- **Retry Logic:**
  - Maximum attempts: 10
  - Wait interval: 60 seconds
  - Total timeout: 600 seconds (10 minutes)
- **Behavior:**
  - If lock exists: Sleep 60s, retry
  - If max attempts exceeded: Return failure
  - If no lock: Proceed with upload

**Upload Execution:**
- **Script:** `$RDK_PATH/uploadSTBLogs.sh`
- **Method:** `fork()` + `execl()` or `system()`
- **Parameters:**
  1. `LOG_SERVER` - Server URL
  2. `1` - Flag parameter
  3. `1` - Flag parameter
  4. `0` - Flag parameter
  5. `UPLOAD_PROTOCOL` - HTTP or HTTPS
  6. `HTTP_UPLOAD_LINK` - Upload endpoint URL
  7. `0` - Flag parameter
  8. `1` - Flag parameter
  9. `UPLOAD_DEBUG_FILE` - Archive filename
- **Working Directory:** `/tmp/rrd/`
- **Return Code:** Capture and check uploadSTBLogs.sh exit status

**Cleanup Operations:**
- **On Success:**
  - Remove archive file
  - Remove source directory recursively
  - Log success message
- **On Failure:**
  - Remove archive file
  - Remove source directory recursively
  - Log failure message
  - Return error code

**Error Handling:**
- Lock timeout → Return failure, cleanup
- Script execution failure → Log error, cleanup
- Cleanup errors → Log but don't fail if primary operation succeeded

**Dependencies:**
- System Info Provider (file operations)
- Process execution functions
- File system operations

---

### 3.7 Logging Subsystem

**Module Name:** `rrd_log`

**Purpose:** Centralized logging for all operations

**Responsibilities:**
- Write timestamped log entries
- Format log messages consistently
- Handle log file rotation (if needed)
- Thread-safe logging (if multi-threaded)

**Key Functions:**
```c
int rrd_log_init(const char *log_path);
void rrd_log_write(const char *format, ...);
void rrd_log_error(const char *format, ...);
void rrd_log_cleanup(void);
```

**Log Format:**
```
[TIMESTAMP]: uploadRRDLogs: [LEVEL] message
Example: 2025-12-01-03-45-30PM: uploadRRDLogs: [INFO] Starting log upload for CRASH_REPORT
```

**Log Levels:**
- INFO: Normal operations
- WARN: Warnings, non-critical issues
- ERROR: Error conditions
- DEBUG: Detailed debugging (optional, compile-time)

**Log File:**
- Path: `$LOG_PATH/remote-debugger.log`
- Append mode
- Create if doesn't exist
- No automatic rotation (handled externally)

**Implementation:**
- Use `fprintf()` or `syslog()` (configurable)
- Mutex protection if multi-threaded
- Buffer flushing for critical messages

**Dependencies:**
- System Info Provider (timestamp)
- File I/O

---

## 4. Data Flow

### 4.1 Primary Data Flow

```
┌──────────────────┐
│ Command Line     │
│ (UPLOADDIR,      │
│  ISSUETYPE)      │
└────────┬─────────┘
         │
         ▼
┌────────────────────────────────────────────────────┐
│ 1. Argument Validation                              │
│    - Check argc == 3                                │
│    - Validate paths                                 │
└────────┬───────────────────────────────────────────┘
         │
         ▼
┌────────────────────────────────────────────────────┐
│ 2. Configuration Loading                            │
│    - Load include.properties                        │
│    - Load device.properties                         │
│    - Query TR-181 (if available)                    │
│    - Parse DCMSettings.conf                         │
│    - Fallback to dcm.properties                     │
└────────┬───────────────────────────────────────────┘
         │
         ▼
┌────────────────────────────────────────────────────┐
│ 3. System Info Gathering                            │
│    - Get MAC address                                │
│    - Generate timestamp                             │
│    - Validate paths                                 │
└────────┬───────────────────────────────────────────┘
         │
         ▼
┌────────────────────────────────────────────────────┐
│ 4. Log Processing                                   │
│    - Validate source directory                      │
│    - Convert issue type to uppercase                │
│    - Handle LOGUPLOAD_ENABLE special case           │
└────────┬───────────────────────────────────────────┘
         │
         ▼
┌────────────────────────────────────────────────────┐
│ 5. Archive Creation                                 │
│    - Generate filename                              │
│    - Create tar.gz archive                          │
│    - Validate archive created                       │
└────────┬───────────────────────────────────────────┘
         │
         ▼
┌────────────────────────────────────────────────────┐
│ 6. Upload Coordination                              │
│    - Check for upload lock                          │
│    - Wait/retry if locked                           │
│    - Execute uploadSTBLogs.sh                       │
│    - Monitor upload status                          │
└────────┬───────────────────────────────────────────┘
         │
         ▼
┌────────────────────────────────────────────────────┐
│ 7. Cleanup                                          │
│    - Remove archive file                            │
│    - Remove source directory                        │
│    - Close log file                                 │
│    - Return status code                             │
└────────────────────────────────────────────────────┘
```

### 4.2 Configuration Data Flow

```
┌──────────────────────────────────────────────────────┐
│ Configuration Sources (Priority Order)               │
└──────────────────────────────────────────────────────┘
         │
         ▼
    ┌────────────────────────────────────────┐
    │ Is BUILD_TYPE == "prod" AND            │
    │ /opt/dcm.properties exists?            │
    └──┬────────────────────────────────┬────┘
       │ YES                            │ NO
       │                                │
       ▼                                ▼
┌──────────────────┐          ┌──────────────────────┐
│ Use ONLY         │          │ Try TR-181 RFC       │
│ /opt/dcm.props   │          │ (if tr181 available) │
│ (Override RFC)   │          └──────────┬───────────┘
└──────┬───────────┘                     │
       │                                 ▼
       │                    ┌────────────────────────┐
       │                    │ Query RFC Parameters:  │
       │                    │ - LogServerUrl         │
       │                    │ - SsrUrl               │
       │                    └──────────┬─────────────┘
       │                               │
       │                               ▼
       │                    ┌────────────────────────┐
       │                    │ Parse DCMSettings.conf │
       │                    │ - UploadRepository:URL │
       │                    │ - uploadProtocol       │
       │                    └──────────┬─────────────┘
       │                               │
       │              ┌────────────────┘
       ▼              ▼
┌───────────────────────────────┐
│ Merge Configuration:          │
│ - LOG_SERVER                  │
│ - HTTP_UPLOAD_LINK            │
│ - UPLOAD_PROTOCOL             │
│ - RDK_PATH                    │
│ - LOG_PATH                    │
└───────┬───────────────────────┘
        │
        ▼
┌───────────────────────────────┐
│ Validate Required Values:     │
│ - LOG_SERVER not empty        │
│ - HTTP_UPLOAD_LINK not empty  │
│ - Set defaults if missing     │
└───────┬───────────────────────┘
        │
        ▼
┌───────────────────────────────┐
│ Configuration Ready           │
└───────────────────────────────┘
```

### 4.3 Error Flow

```
┌─────────────────┐
│ Error Detected  │
└────────┬────────┘
         │
         ▼
┌──────────────────────────────┐
│ Log Error with Context        │
│ - Timestamp                   │
│ - Error code                  │
│ - Error message               │
└────────┬─────────────────────┘
         │
         ▼
┌──────────────────────────────┐
│ Attempt Cleanup               │
│ - Remove partial files        │
│ - Close open handles          │
│ - Release resources           │
└────────┬─────────────────────┘
         │
         ▼
┌──────────────────────────────┐
│ Return Error Code             │
│ - 1: Argument error           │
│ - 2: Config error             │
│ - 3: Archive error            │
│ - 4: Upload error             │
│ - Other: Script-defined       │
└───────────────────────────────┘
```

## 5. Key Algorithms and Data Structures

### 5.1 Configuration Parser Algorithm

**Purpose:** Parse key=value property files

**Algorithm:**
```
FUNCTION parse_property_file(filepath, config_struct):
    OPEN file for reading
    IF file not accessible THEN
        RETURN error_code
    END IF
    
    FOR each line in file:
        TRIM whitespace
        IF line is empty OR starts with '#' THEN
            CONTINUE  // Skip comments and empty lines
        END IF
        
        SPLIT line on '=' into key and value
        IF split failed THEN
            LOG warning "Malformed line"
            CONTINUE
        END IF
        
        REMOVE quotes from value if present
        TRIM whitespace from key and value
        
        MATCH key:
            CASE "LOG_SERVER":
                COPY value to config_struct.log_server
            CASE "HTTP_UPLOAD_LINK":
                COPY value to config_struct.http_upload_link
            CASE "UPLOAD_PROTOCOL":
                COPY value to config_struct.upload_protocol
            // ... other cases
        END MATCH
    END FOR
    
    CLOSE file
    RETURN success
END FUNCTION
```

**Data Structure:**
```c
typedef struct {
    char log_server[256];
    char http_upload_link[512];
    char upload_protocol[16];
    char rdk_path[256];
    char log_path[256];
    char build_type[32];
    bool use_rfc_config;
} rrd_config_t;
```

### 5.2 MAC Address Retrieval Algorithm

**Purpose:** Get device MAC address using multiple fallback methods

**Algorithm:**
```
FUNCTION get_mac_address(output_buffer, buffer_size):
    // Method 1: Read from sysfs
    interface_list = ["eth0", "wlan0", "erouter0"]
    FOR each interface in interface_list:
        path = "/sys/class/net/" + interface + "/address"
        IF file_exists(path) THEN
            READ mac_address from path
            IF mac_address is valid THEN
                COPY to output_buffer
                RETURN success
            END IF
        END IF
    END FOR
    
    // Method 2: ioctl
    FOR each interface in interface_list:
        socket = socket(AF_INET, SOCK_DGRAM, 0)
        IF socket valid THEN
            SET ifr.ifr_name = interface
            IF ioctl(socket, SIOCGIFHWADDR, &ifr) succeeds THEN
                FORMAT MAC from ifr.ifr_hwaddr.sa_data
                COPY to output_buffer
                CLOSE socket
                RETURN success
            END IF
            CLOSE socket
        END IF
    END FOR
    
    // Method 3: Execute getMacAddressOnly (compatibility)
    EXECUTE "$RDK_PATH/utils.sh getMacAddressOnly"
    IF execution succeeds AND output valid THEN
        COPY output to buffer
        RETURN success
    END IF
    
    RETURN error_no_mac_found
END FUNCTION
```

### 5.3 Upload Lock Management Algorithm

**Purpose:** Prevent concurrent uploads with retry logic

**Algorithm:**
```
FUNCTION wait_for_upload_lock(max_attempts, wait_seconds):
    attempt = 1
    
    WHILE attempt <= max_attempts:
        IF NOT file_exists("/tmp/.log-upload.pid") THEN
            // Lock is free
            RETURN success
        ELSE
            // Lock held by another process
            LOG "Upload lock detected, waiting..."
            SLEEP wait_seconds
            attempt = attempt + 1
        END IF
    END WHILE
    
    // Max attempts exceeded
    LOG "Upload lock timeout after max attempts"
    RETURN error_lock_timeout
END FUNCTION

FUNCTION execute_upload_with_lock(parameters):
    result = wait_for_upload_lock(10, 60)
    IF result != success THEN
        RETURN result
    END IF
    
    // Lock is free, proceed with upload
    result = invoke_upload_script(parameters)
    RETURN result
END FUNCTION
```

### 5.4 Archive Filename Generation Algorithm

**Purpose:** Generate standardized archive filename

**Algorithm:**
```
FUNCTION generate_archive_filename(mac, issue_type, timestamp, output_buffer, size):
    // Sanitize MAC: remove colons
    sanitized_mac = REPLACE(mac, ":", "")
    
    // Ensure issue_type is uppercase
    uppercase_issue = TO_UPPERCASE(issue_type)
    
    // Format: {MAC}_{ISSUETYPE}_{TIMESTAMP}_RRD_DEBUG_LOGS.tgz
    result = SNPRINTF(output_buffer, size,
                      "%s_%s_%s_RRD_DEBUG_LOGS.tgz",
                      sanitized_mac,
                      uppercase_issue,
                      timestamp)
    
    IF result < 0 OR result >= size THEN
        RETURN error_buffer_too_small
    END IF
    
    RETURN success
END FUNCTION
```

### 5.5 Directory Validation Algorithm

**Purpose:** Validate source directory for archiving

**Algorithm:**
```
FUNCTION validate_source_directory(dir_path):
    // Check existence
    IF NOT dir_exists(dir_path) THEN
        LOG "Directory does not exist: " + dir_path
        RETURN error_not_found
    END IF
    
    // Check readable
    IF NOT dir_readable(dir_path) THEN
        LOG "Directory not readable: " + dir_path
        RETURN error_permission
    END IF
    
    // Check not empty
    IF dir_is_empty(dir_path) THEN
        LOG "Directory is empty: " + dir_path
        RETURN error_empty_directory
    END IF
    
    // Check disk space in /tmp
    available_space = get_available_space("/tmp")
    dir_size = get_directory_size(dir_path)
    required_space = dir_size * 1.2  // 20% overhead for compression
    
    IF available_space < required_space THEN
        LOG "Insufficient disk space"
        RETURN error_insufficient_space
    END IF
    
    RETURN success
END FUNCTION
```

## 6. Interfaces and Integration Points

### 6.1 Command-Line Interface

**Synopsis:**
```
uploadRRDLogs UPLOADDIR ISSUETYPE
```

**Arguments:**
- `UPLOADDIR`: Path to directory containing debug logs
- `ISSUETYPE`: Issue classification string

**Exit Codes:**
- `0`: Success
- `1`: Invalid arguments
- `2`: Configuration error
- `3`: Archive creation error
- `4`: Upload error
- `5`: Cleanup error

**Environment Variables Required:**
- `RDK_PATH`: Path to RDK utilities
- `LOG_PATH`: Path for log files

**Example Usage:**
```bash
export RDK_PATH=/lib/rdk
export LOG_PATH=/opt/logs
./uploadRRDLogs /tmp/rrd_logs/ crash_report
```

### 6.2 File System Interface

**Input Files:**
| Path | Purpose | Required | Format |
|------|---------|----------|--------|
| /etc/include.properties | System properties | Yes | key=value |
| /etc/device.properties | Device config | Yes | key=value |
| /tmp/DCMSettings.conf | DCM settings | No | key=value |
| /opt/dcm.properties | DCM fallback | Conditional | key=value |
| /etc/dcm.properties | DCM fallback | Conditional | key=value |

**Output Files:**
| Path | Purpose | Lifetime |
|------|---------|----------|
| /tmp/rrd/{archive}.tgz | Log archive | Temporary (deleted after upload) |
| $LOG_PATH/remote-debugger.log | Operation log | Persistent (external rotation) |

**Working Directories:**
| Path | Purpose |
|------|---------|
| /tmp/rrd/ | Archive creation workspace |
| {UPLOADDIR} | Source log files |

### 6.3 External Script Interface

**uploadSTBLogs.sh Interface:**

**Invocation:**
```bash
$RDK_PATH/uploadSTBLogs.sh \
    "$LOG_SERVER" \
    1 \
    1 \
    0 \
    "$UPLOAD_PROTOCOL" \
    "$HTTP_UPLOAD_LINK" \
    0 \
    1 \
    "$UPLOAD_DEBUG_FILE"
```

**Parameters:**
1. Log server URL
2. Flag (always 1)
3. Flag (always 1)
4. Flag (always 0)
5. Upload protocol (HTTP/HTTPS)
6. HTTP upload endpoint URL
7. Flag (always 0)
8. Flag (always 1)
9. Archive filename

**Expected Behavior:**
- Creates `/tmp/.log-upload.pid` lock file
- Uploads archive to remote server
- Removes lock file upon completion
- Returns 0 on success, non-zero on failure

**Working Directory:**
- Must be executed from `/tmp/rrd/`

### 6.4 TR-181 Interface

**tr181 Tool Interface:**

**Invocation:**
```bash
/usr/bin/tr181 -g <parameter_path>
```

**Parameters to Query:**
- `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.LogServerUrl`
- `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.SsrUrl`

**Expected Output:**
- Single line with parameter value
- Empty string if parameter not set
- Error message to stderr on failure

**Error Handling:**
- Tool not present: Skip RFC query
- Query fails: Use DCM configuration fallback
- Empty result: Use DCM configuration fallback

### 6.5 Logging Interface

**Log File:** `$LOG_PATH/remote-debugger.log`

**Log Entry Format:**
```
YYYY-MM-DD-HH-MM-SS[AM|PM]: uploadRRDLogs: [LEVEL] message
```

**Example Entries:**
```
2025-12-01-03-45-30PM: uploadRRDLogs: [INFO] Starting log upload for CRASH_REPORT
2025-12-01-03-45-32PM: uploadRRDLogs: [INFO] Archive created: 112233445566_CRASH_REPORT_2025-12-01-03-45-30PM_RRD_DEBUG_LOGS.tgz
2025-12-01-03-46-15PM: uploadRRDLogs: [INFO] Upload successful
2025-12-01-03-46-16PM: uploadRRDLogs: [ERROR] Failed to remove archive file: Permission denied
```

**Log Levels:**
- `[INFO]`: Normal operational messages
- `[WARN]`: Warning conditions
- `[ERROR]`: Error conditions
- `[DEBUG]`: Debugging information (optional)

## 7. Error Handling Strategy

### 7.1 Error Categories

**Category 1: Fatal Errors (Immediate Exit)**
- Invalid command-line arguments
- Critical configuration missing (LOG_SERVER, HTTP_UPLOAD_LINK)
- Unable to initialize logging
- Out of memory

**Category 2: Recoverable Errors (Retry/Fallback)**
- TR-181 query failure → Fall back to DCM config
- uploadSTBLogs.sh busy → Retry with timeout
- Temporary file I/O error → Retry operation

**Category 3: Warning Conditions (Log and Continue)**
- Optional configuration file missing
- RRD_LIVE_LOGS.tar.gz not found for LOGUPLOAD_ENABLE
- Non-critical file in source directory unreadable

### 7.2 Error Codes

| Code | Category | Description |
|------|----------|-------------|
| 0 | Success | Operation completed successfully |
| 1 | Fatal | Invalid command-line arguments |
| 2 | Fatal | Configuration error |
| 3 | Fatal | Archive creation failed |
| 4 | Recoverable | Upload failed |
| 5 | Warning | Cleanup failed (after successful upload) |

### 7.3 Error Handling Pattern

```c
int function_that_can_fail() {
    int result;
    
    // Attempt operation
    result = risky_operation();
    if (result != 0) {
        // Log error with context
        rrd_log_error("Operation failed: %s (errno=%d)", 
                      strerror(errno), errno);
        
        // Attempt cleanup
        cleanup_partial_work();
        
        // Return error code
        return ERROR_OPERATION_FAILED;
    }
    
    return SUCCESS;
}
```

### 7.4 Resource Cleanup Pattern

```c
int main_function() {
    resource_t *res1 = NULL;
    resource_t *res2 = NULL;
    int result = SUCCESS;
    
    // Allocate resources
    res1 = allocate_resource1();
    if (res1 == NULL) {
        result = ERROR_ALLOCATION;
        goto cleanup;
    }
    
    res2 = allocate_resource2();
    if (res2 == NULL) {
        result = ERROR_ALLOCATION;
        goto cleanup;
    }
    
    // Perform operations
    result = do_work(res1, res2);
    
cleanup:
    // Always clean up resources
    if (res2 != NULL) {
        free_resource2(res2);
    }
    if (res1 != NULL) {
        free_resource1(res1);
    }
    
    return result;
}
```

## 8. Performance Considerations

### 8.1 Memory Usage Optimization

**Techniques:**
1. **Stack Allocation:** Use stack for small, fixed-size buffers
2. **Buffer Reuse:** Reuse buffers across operations
3. **Streaming:** Process files without loading into memory
4. **Early Cleanup:** Free resources as soon as no longer needed

**Memory Budget:**
- Configuration data: ~2 KB
- Path buffers: ~2 KB
- Archive operations: Streaming (minimal memory)
- Total target: < 100 KB resident memory

### 8.2 I/O Optimization

**Strategies:**
1. **Sequential Reads:** Read files sequentially for tar operations
2. **Buffered I/O:** Use appropriate buffer sizes (8-64 KB)
3. **Minimize Seeks:** Avoid random access patterns
4. **Batch Operations:** Group file operations where possible

### 8.3 CPU Optimization

**Techniques:**
1. **Efficient String Operations:** Use memcpy/memmove instead of strcpy for known lengths
2. **Avoid Redundant Parsing:** Parse configuration once, cache results
3. **Minimize System Calls:** Batch operations where possible
4. **Compression:** Let tar/gzip handle compression (efficient implementation)

### 8.4 Execution Time Targets

| Operation | Target Time | Notes |
|-----------|-------------|-------|
| Initialization | < 1 second | Config loading, validation |
| Archive Creation | Variable | Depends on log size (~5 MB/s) |
| Upload | Variable | Network-dependent |
| Cleanup | < 1 second | File deletion |
| Total (100 MB logs) | < 5 minutes | Excludes network upload time |

## 9. Security Considerations

### 9.1 Input Validation

**Command-Line Arguments:**
- Validate UPLOADDIR path doesn't contain `..` (directory traversal)
- Sanitize ISSUETYPE for filesystem safety
- Check argument lengths to prevent buffer overflows

**Configuration Values:**
- Validate URLs for proper format
- Check path values for dangerous characters
- Limit string lengths

### 9.2 File Operations

**Secure Practices:**
- Create temporary files with mode 0600 (owner read/write only)
- Use absolute paths to prevent race conditions
- Validate symbolic links before following
- Check file permissions before operations

### 9.3 Process Execution

**Security Measures:**
- Use `execv()` family instead of `system()` where possible
- Sanitize arguments passed to external scripts
- Set minimal environment for child processes
- Validate executables before execution (check permissions, ownership)

### 9.4 Sensitive Data Handling

**Considerations:**
- MAC address in filename: Consider hashing if privacy required
- Log contents: May contain sensitive debug information
- Credentials: Never log server credentials
- Temporary files: Clean up promptly, use secure permissions

### 9.5 Network Security

**Best Practices:**
- Prefer HTTPS over HTTP for uploads
- Validate server certificates (if implementing TLS)
- Use timeout for network operations
- Avoid credential exposure in URLs

## 10. Platform Portability

### 10.1 POSIX Compliance

**Required Standards:**
- POSIX.1-2008 (base specification)
- C99 (minimum), C11 (preferred)
- Large File Support (LFS) for files > 2GB

**System Calls:**
- Use POSIX-compliant functions
- Avoid Linux-specific extensions where possible
- Provide fallbacks for optional features

### 10.2 Cross-Platform Considerations

**File Paths:**
- Use `/` as path separator (POSIX standard)
- Support paths up to PATH_MAX (4096 bytes)
- Handle long filenames (NAME_MAX = 255)

**Endianness:**
- Not a concern (text-based protocols and files)
- Binary operations: Use network byte order if needed

**Architecture:**
- Support 32-bit and 64-bit systems
- Use standard integer types (int32_t, uint64_t, etc.)
- Avoid architecture-specific assumptions

### 10.3 Compiler Compatibility

**Target Compilers:**
- GCC 4.8+ (primary)
- Clang 3.5+ (secondary)
- Cross-compilers: ARM, MIPS, x86 variants

**Compilation Flags:**
- `-std=c99` or `-std=c11`
- `-Wall -Wextra -Werror` (strict warnings)
- `-Os` (optimize for size on embedded platforms)
- `-D_LARGEFILE64_SOURCE` (large file support)

### 10.4 Build System

**Autotools Configuration:**
```
configure.ac additions:
- Check for libarchive (optional)
- Check for tr181 tool
- Detect system properties files locations
- Support cross-compilation
```

**Makefile.am:**
```makefile
bin_PROGRAMS = uploadRRDLogs
uploadRRDLogs_SOURCES = rrd_main.c rrd_config.c rrd_sysinfo.c \
                        rrd_logproc.c rrd_archive.c rrd_upload.c \
                        rrd_log.c
uploadRRDLogs_CFLAGS = -Wall -Wextra -Os -std=c99
uploadRRDLogs_LDFLAGS = -larchive (optional)
```

## 11. Dependencies Matrix

| Module | Depends On | External Libraries | System Calls |
|--------|------------|-------------------|--------------|
| rrd_main | All modules | stdlib, stdio | - |
| rrd_config | rrd_sysinfo, rrd_log | string | fopen, fgets, popen |
| rrd_sysinfo | rrd_log | string, time, net | stat, access, opendir, ioctl |
| rrd_logproc | rrd_sysinfo, rrd_log | string | stat, opendir |
| rrd_archive | rrd_sysinfo, rrd_log | libarchive (opt) | system/fork/exec |
| rrd_upload | rrd_sysinfo, rrd_log | - | access, system/fork/exec |
| rrd_log | - | stdarg, time | fopen, fprintf |

**Optional Dependencies:**
- **libarchive:** For native C tar creation (preferred)
- **tr181 tool:** For RFC parameter queries
- **uploadSTBLogs.sh:** External script (required)

**Fallback Strategies:**
- No libarchive → Use system tar command
- No tr181 → Use DCM configuration only
- uploadSTBLogs.sh unavailable → Fatal error (required)

## 12. Testing Strategy

### 12.1 Unit Testing

**Test Framework:** Google Test (gtest/gmock)

**Modules to Test:**
- Configuration parser (various file formats)
- MAC address retrieval (with mocked interfaces)
- Timestamp generation
- Filename generation
- Directory validation
- Lock management logic

**Test Coverage Target:** > 80%

### 12.2 Integration Testing

**Test Scenarios:**
1. End-to-end successful upload
2. Configuration fallback chain
3. Lock file contention (multiple instances)
4. Empty directory handling
5. Archive creation for various log sizes
6. Upload failure and retry
7. Cleanup on success and failure

### 12.3 Platform Testing

**Target Platforms:**
- ARM 32-bit embedded Linux
- ARM 64-bit embedded Linux
- MIPS embedded Linux
- x86_64 Linux (development)

**Validation:**
- Memory usage profiling (valgrind, massif)
- Binary size check (< 500KB preferred)
- Performance benchmarking
- Resource leak detection

### 12.4 Error Injection Testing

**Inject Errors:**
- Missing configuration files
- Disk full conditions
- Network unavailability
- Permission errors
- Malformed input data
- Lock file timeout

## 13. Migration Compatibility

### 13.1 Behavioral Compatibility

**Must Maintain:**
- Identical command-line interface
- Same exit codes for common scenarios
- Log format compatibility
- Archive naming convention
- Integration with uploadSTBLogs.sh

**Allowed Changes:**
- Performance improvements
- Better error messages
- Additional debug logging (if enabled)

### 13.2 Configuration Compatibility

**Must Support:**
- All property file formats
- TR-181 RFC parameters
- DCM configuration sources
- Priority and fallback order

**Deprecated:**
- None (all features retained)

### 13.3 Deployment Strategy

**Phase 1: Side-by-Side Testing**
- Install C version as `uploadRRDLogs_c`
- Run both versions in parallel
- Compare outputs and logs

**Phase 2: Gradual Rollout**
- Deploy to subset of devices
- Monitor for issues
- Collect performance metrics

**Phase 3: Full Replacement**
- Replace shell script with C binary
- Update systemd units / init scripts
- Remove shell script dependency

## Document Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | December 1, 2025 | GitHub Copilot | Initial HLD document |
