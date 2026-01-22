# uploadRRDLogs.sh - Requirements Document

## Document Information
- **Script Name:** uploadRRDLogs.sh
- **Target Migration:** C Program
- **Version:** 1.0
- **Date:** December 1, 2025

## 1. Functional Requirements

### 1.1 Core Functionality
The script is responsible for collecting, packaging, and uploading Remote Debugger (RRD) logs to a remote log server for analysis and troubleshooting purposes.

### 1.2 Primary Operations

#### FR-1: Command-Line Argument Validation
- **Requirement:** Accept exactly 2 command-line arguments
- **Arguments:**
  - `UPLOADDIR`: Directory path containing debug logs to upload
  - `ISSUETYPE`: Type/category of issue being reported
- **Validation:** Exit with error code 1 if argument count is invalid
- **Error Message:** Display usage information when validation fails

#### FR-2: Configuration Loading
- **Requirement:** Load configuration from multiple property files
- **Property Files:**
  - `/etc/include.properties` - Common RDK properties
  - `/etc/device.properties` - Device-specific properties
  - `$RDK_PATH/utils.sh` - Utility functions
  - `$OUTFILE` (/tmp/DCMSettings.conf) - DCM configuration
  - `/opt/dcm.properties` or `/etc/dcm.properties` - DCM fallback configuration
- **Behavior:** Fail gracefully if critical configuration is missing

#### FR-3: System Information Gathering
- **Requirement:** Collect system identification information
- **Information Required:**
  - MAC address (using `getMacAddressOnly` function)
  - Timestamp in format: YYYY-MM-DD-HH-MM-SS{AM/PM}
  - Build type (prod vs non-prod)
- **Purpose:** Generate unique identifiers for uploaded logs

#### FR-4: Upload Server Configuration
- **Requirement:** Determine upload destination and protocol
- **Configuration Sources (in priority order):**
  1. RFC parameters (via tr181 interface):
     - `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.LogServerUrl`
     - `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.SsrUrl`
  2. DCMSettings.conf:
     - `LogUploadSettings:UploadRepository:URL`
     - `LogUploadSettings:UploadRepository:uploadProtocol`
  3. DCM properties files (fallback)
- **Default Protocol:** HTTP if not specified
- **Validation:** Ensure LOG_SERVER and HTTP_UPLOAD_LINK are non-empty

#### FR-5: Log Archive Creation
- **Requirement:** Create compressed tar archive of debug logs
- **Archive Naming:** `{MAC}_{ISSUETYPE}_{TIMESTAMP}_RRD_DEBUG_LOGS.tgz`
- **Content Source:** All files in `$RRD_LOG_PATH` directory
- **Working Directory:** `/tmp/rrd/`
- **Compression:** gzip compression (tar -zcf)
- **Special Case:** For LOGUPLOAD_ENABLE issue type, include `RRD_LIVE_LOGS.tar.gz`

#### FR-6: Concurrent Upload Protection
- **Requirement:** Prevent multiple simultaneous upload operations
- **Mechanism:** Check for existence of `/tmp/.log-upload.pid` file
- **Behavior:** 
  - Wait up to 10 attempts (60 seconds between attempts)
  - Proceed with upload if lock file is not present
  - Return failure if max attempts exceeded
- **Purpose:** Avoid conflicts and resource contention

#### FR-7: Log Upload Execution
- **Requirement:** Upload log archive to remote server
- **Upload Script:** `$RDK_PATH/uploadSTBLogs.sh`
- **Parameters Passed:**
  - Log server URL
  - Upload flags: 1 1 0 (specific to uploadSTBLogs.sh)
  - Upload protocol (HTTP/HTTPS)
  - HTTP upload link
  - Flag: 0 1 (additional uploadSTBLogs.sh flags)
  - Archive filename
- **Return Value:** Upload success/failure status

#### FR-8: Cleanup Operations
- **Requirement:** Remove temporary files after upload
- **Cleanup Actions:**
  - On Success: Delete archive file and source directory
  - On Failure: Delete archive file and source directory
  - Always: Clean up `/tmp/rrd/` working directory
- **Files to Remove:**
  - `$UPLOAD_DEBUG_FILE` (tar archive)
  - `$RRD_LOG_PATH` (source directory)

#### FR-9: Logging and Audit Trail
- **Requirement:** Log all operations for troubleshooting
- **Log File:** `$LOG_PATH/remote-debugger.log`
- **Log Format:** `[TIMESTAMP]: scriptname: message`
- **Logged Events:**
  - Script execution start
  - Configuration reading operations
  - Archive creation
  - Upload attempts and results
  - Cleanup operations
  - Error conditions

#### FR-10: Error Handling
- **Requirement:** Handle error conditions gracefully
- **Error Scenarios:**
  - Invalid command-line arguments
  - Missing configuration files
  - Empty or missing source directory
  - Upload failures
  - File system errors
- **Behavior:** Log errors and exit with appropriate status codes

### 1.3 Issue Type Handling

#### FR-11: Special Issue Type Processing
- **Issue Type:** LOGUPLOAD_ENABLE
- **Special Action:** Include live device logs (`RRD_LIVE_LOGS.tar.gz`)
- **Behavior:** Move live logs file into upload directory before archiving
- **Case Handling:** Issue type is converted to uppercase for processing

## 2. Input Specifications

### 2.1 Command-Line Arguments
- **Argument 1 (UPLOADDIR):** 
  - Type: String (directory path)
  - Format: Absolute or relative path
  - Validation: Must be a valid directory containing files
  - Example: `/tmp/rrd_logs/`

- **Argument 2 (ISSUETYPE):**
  - Type: String
  - Format: Alphanumeric identifier
  - Processing: Converted to uppercase
  - Example: `logupload_enable`, `crash_report`, `performance_issue`

### 2.2 Configuration Files
- **include.properties:** RDK system variables
- **device.properties:** Device-specific configuration
- **DCMSettings.conf:** Dynamic configuration from DCM server
- **dcm.properties:** Static DCM configuration

### 2.3 Environment Variables
- `RDK_PATH`: Base path for RDK utilities and scripts
- `LOG_PATH`: Path for log files
- `BUILD_TYPE`: Build type (prod/dev/etc.)

### 2.4 File System Inputs
- Debug log files in `$RRD_LOG_PATH` directory
- Optional: `RRD_LIVE_LOGS.tar.gz` for live logs

## 3. Output Specifications

### 3.1 Primary Output
- **Compressed Archive:** `{MAC}_{ISSUETYPE}_{TIMESTAMP}_RRD_DEBUG_LOGS.tgz`
- **Location:** `/tmp/rrd/`
- **Format:** gzip-compressed tar archive
- **Contents:** All files from source directory

### 3.2 Log Output
- **Log File:** `$LOG_PATH/remote-debugger.log`
- **Format:** Timestamped entries with script name and message
- **Content:** Operational events, errors, and status information

### 3.3 Exit Codes
- **0:** Successful upload and cleanup
- **1:** Invalid arguments or critical error
- **Non-zero:** Upload failure (specific code from uploadSTBLogs.sh)

### 3.4 Network Output
- **Upload:** HTTP/HTTPS POST to remote log server
- **Destination:** Configured log server URL
- **Protocol:** HTTP or HTTPS based on configuration

## 4. Dependencies

### 4.1 External Scripts
- **utils.sh:** Provides utility functions
  - `getMacAddressOnly()`: Retrieves device MAC address
  - `timestamp`: Generates formatted timestamps
  - Other utility functions

- **uploadSTBLogs.sh:** Handles actual upload operation
  - Parameters: Server URL, flags, protocol, file
  - Returns: Upload success/failure status

### 4.2 System Commands
- `/bin/sh`: Shell interpreter
- `date`: Timestamp generation
- `tr`: Text transformation (case conversion)
- `tar`: Archive creation (with gzip)
- `cat`: File reading
- `grep`: Pattern matching
- `cut`: Field extraction
- `sed`: Text substitution
- `mv`: File moving
- `rm`: File removal
- `cd`: Directory navigation
- `ls`: Directory listing

### 4.3 Optional Dependencies
- **tr181:** RFC parameter retrieval tool
  - Used for reading Device.DeviceInfo.X_RDKCENTRAL-COM_RFC parameters
  - Available only on systems with TR-181 support

### 4.4 File System Dependencies
- `/etc/include.properties`: Required
- `/etc/device.properties`: Required
- `/tmp/DCMSettings.conf`: Optional (runtime generated)
- `/opt/dcm.properties` or `/etc/dcm.properties`: Fallback required
- `/tmp/.log-upload.pid`: Lock file for concurrent access
- `/tmp/rrd/`: Working directory
- `$LOG_PATH/remote-debugger.log`: Log file

## 5. Constraints

### 5.1 Memory Constraints
- **Target Platforms:** Embedded systems with limited RAM (few KB to few MB)
- **Requirements:**
  - Minimize dynamic memory allocation
  - Use fixed-size buffers where possible
  - Avoid large temporary data structures
  - Stream processing for large files
- **Considerations:**
  - Archive creation should not load entire contents into memory
  - Configuration parsing should use minimal buffers

### 5.2 Timing Constraints
- **Upload Retry:** Maximum 10 attempts with 60-second intervals
- **Total Wait Time:** Up to 600 seconds (10 minutes) for upload lock
- **No Hard Real-Time:** Soft real-time acceptable for log upload operations
- **Background Operation:** Should not block critical system operations

### 5.3 Resource Constraints
- **CPU:** Low-power embedded processors
  - Minimize CPU-intensive operations
  - Use efficient algorithms for string processing
  - Avoid complex computations
- **Storage:** Limited temporary storage
  - Clean up archives after upload
  - Remove source files after successful upload
  - Use `/tmp` for temporary files

### 5.4 Platform Constraints
- **Portability:** Must work across multiple embedded platforms
- **Architecture:** ARM, MIPS, x86 (various architectures)
- **OS:** Linux-based embedded systems
- **Compiler:** GCC with C99 standard minimum
- **Cross-Compilation:** Must support cross-compilation toolchains

### 5.5 Network Constraints
- **Protocol Support:** HTTP and HTTPS
- **Timeout Handling:** Handle network timeouts gracefully
- **Retry Logic:** Already handled by uploadSTBLogs.sh (external)
- **Bandwidth:** Efficient upload of potentially large log archives

### 5.6 Concurrency Constraints
- **Single Instance:** Only one upload operation at a time
- **Lock File:** `/tmp/.log-upload.pid` prevents concurrent uploads
- **Thread Safety:** Not required for single-threaded operation
- **Process Isolation:** Each invocation is independent

## 6. Edge Cases and Error Handling

### 6.1 Edge Case: Empty Directory
- **Scenario:** `$RRD_LOG_PATH` exists but contains no files
- **Detection:** `ls -A $RRD_LOG_PATH` returns empty
- **Handling:** Log message and exit without attempting upload
- **Log Message:** "$RRD_LOG_PATH is Empty, Exiting!!!"

### 6.2 Edge Case: Missing Directory
- **Scenario:** `$RRD_LOG_PATH` does not exist
- **Detection:** Directory test fails
- **Handling:** Log message and exit gracefully
- **Log Message:** "$RRD_LOG_PATH is Empty, Exiting!!!"

### 6.3 Edge Case: Missing Configuration
- **Scenario:** Required configuration files are missing
- **Handling:** 
  - Try RFC/tr181 parameters first
  - Fall back to DCMSettings.conf
  - Fall back to dcm.properties
  - Exit if all sources fail
- **Priority:** Non-prod builds with /opt/dcm.properties override RFC

### 6.4 Edge Case: tr181 Command Not Available
- **Scenario:** System lacks TR-181 support
- **Detection:** `/usr/bin/tr181` file check
- **Handling:** Skip RFC parameter reading, use DCMSettings.conf
- **Graceful Degradation:** Continue with fallback configuration

### 6.5 Edge Case: Upload Lock Timeout
- **Scenario:** Another upload holds lock for > 10 minutes
- **Handling:** Return failure after 10 attempts
- **Cleanup:** Remove local archive and source directory
- **Log:** Record timeout condition

### 6.6 Edge Case: Upload Failure
- **Scenario:** uploadSTBLogs.sh returns non-zero exit code
- **Handling:**
  - Log failure message
  - Clean up archive and source directory
  - Propagate failure exit code
- **Log Message:** "RRD {ISSUETYPE} Debug Information Report upload Failed!!!"

### 6.7 Edge Case: Archive Creation Failure
- **Scenario:** tar command fails (disk full, permissions, etc.)
- **Handling:**
  - Detect tar exit status
  - Log error to remote-debugger.log
  - Clean up partial files
  - Exit with error code

### 6.8 Edge Case: Disk Space Exhaustion
- **Scenario:** Insufficient space for archive creation
- **Detection:** tar command failure
- **Handling:**
  - Log disk space error
  - Clean up any partial files
  - Exit with appropriate error code
- **Prevention:** Check available space before archive creation

### 6.9 Edge Case: Permission Errors
- **Scenario:** Insufficient permissions for file operations
- **Detection:** Command failures (tar, mv, rm)
- **Handling:**
  - Log permission error
  - Attempt cleanup with available permissions
  - Exit with error code

### 6.10 Edge Case: Invalid Issue Type
- **Scenario:** Issue type contains special characters or is malformed
- **Handling:** 
  - Convert to uppercase as-is
  - Allow uploadSTBLogs.sh to handle validation
  - Include in filename (sanitization may be needed)

### 6.11 Edge Case: Very Large Log Directories
- **Scenario:** Log directory contains gigabytes of data
- **Handling:**
  - Stream tar operation (no memory loading)
  - Monitor disk space during archive creation
  - Consider timeout for upload operation
- **Risk Mitigation:** Implement size limits if necessary

### 6.12 Edge Case: Network Unavailability
- **Scenario:** Network is down or unreachable
- **Handling:** 
  - Delegated to uploadSTBLogs.sh
  - Receive failure status
  - Clean up and log failure
  - No retry logic (handled by uploadSTBLogs.sh)

### 6.13 Edge Case: LOGUPLOAD_ENABLE Without Live Logs
- **Scenario:** Issue type is LOGUPLOAD_ENABLE but RRD_LIVE_LOGS.tar.gz missing
- **Handling:**
  - Attempt to move file
  - If mv fails, continue without it
  - Log warning but don't fail entire operation
- **Behavior:** Graceful degradation

### 6.14 Edge Case: Race Condition on Lock File
- **Scenario:** Lock file created between check and upload start
- **Handling:**
  - uploadSTBLogs.sh likely handles this internally
  - If detected, fall into retry loop
  - Wait for lock release with timeout

### 6.15 Edge Case: Extremely Long Paths or Filenames
- **Scenario:** Paths exceed system PATH_MAX limits
- **Handling:**
  - Use appropriate buffer sizes
  - Validate path lengths before operations
  - Return error if paths are too long
- **Prevention:** Define safe maximum path lengths

## 7. Security Considerations

### 7.1 Input Validation
- **Command-Line Arguments:** Validate paths to prevent directory traversal
- **Configuration Values:** Sanitize URLs and paths from configuration files
- **Issue Type:** Sanitize for use in filenames (prevent shell injection)

### 7.2 Secure File Operations
- **Temporary Files:** Create with restrictive permissions (0600 or 0640)
- **Directory Access:** Validate directory existence and permissions before access
- **Path Traversal:** Prevent access to files outside intended directories

### 7.3 Sensitive Data
- **MAC Address:** Consider privacy implications of MAC in filename
- **Log Contents:** Logs may contain sensitive debugging information
- **Credentials:** Ensure no credentials are logged in plaintext

### 7.4 Network Security
- **HTTPS Support:** Prefer HTTPS over HTTP for uploads
- **Certificate Validation:** Validate server certificates (if implemented)
- **URL Validation:** Validate log server URLs to prevent injection

### 7.5 Process Security
- **Lock File:** Prevent race conditions with proper locking
- **Signal Handling:** Handle interrupts gracefully to prevent orphaned files
- **Resource Cleanup:** Always clean up on error paths

### 7.6 Audit Trail
- **Logging:** Maintain complete audit trail in remote-debugger.log
- **Timestamps:** Include accurate timestamps for all operations
- **Status Codes:** Log all error conditions with appropriate context

## 8. Performance Requirements

### 8.1 Execution Time
- **Target:** Complete within 5 minutes under normal conditions
- **Archive Creation:** Depends on log size, should be streaming
- **Upload:** Network-dependent, handled by uploadSTBLogs.sh
- **Overhead:** Minimal CPU time for script logic

### 8.2 Resource Usage
- **Memory:** Peak usage < 1MB for script logic
- **Disk I/O:** Sequential reads for tar operations
- **CPU:** Low CPU usage except during compression
- **Network:** Burst upload traffic

### 8.3 Scalability
- **Log Size:** Handle logs from KB to hundreds of MB
- **File Count:** Support thousands of individual log files
- **Concurrent Instances:** Prevent concurrent execution

## 9. Functional Workflow Summary

1. **Initialization Phase:**
   - Validate command-line arguments
   - Load configuration files
   - Retrieve system information (MAC, timestamp)

2. **Configuration Phase:**
   - Determine upload server from RFC/DCM settings
   - Determine upload protocol
   - Set default values if configuration is missing

3. **Validation Phase:**
   - Verify source directory exists and contains files
   - Convert issue type to uppercase

4. **Preparation Phase:**
   - Handle special case for LOGUPLOAD_ENABLE
   - Move to working directory

5. **Archive Phase:**
   - Create compressed tar archive of logs
   - Use naming convention with MAC, issue type, timestamp

6. **Upload Phase:**
   - Check for concurrent upload lock
   - Wait with retry logic if lock exists
   - Invoke uploadSTBLogs.sh with appropriate parameters

7. **Completion Phase:**
   - Check upload result
   - Log success or failure
   - Clean up archive and source directory
   - Exit with appropriate status code

## 10. Success Criteria

### 10.1 Functional Success
- ✓ All debug logs successfully archived
- ✓ Archive uploaded to remote server
- ✓ Temporary files cleaned up
- ✓ Complete audit trail in log file
- ✓ Appropriate exit code returned

### 10.2 Quality Success
- ✓ No memory leaks
- ✓ Proper error handling for all failure modes
- ✓ Graceful degradation when optional features unavailable
- ✓ Clear, actionable error messages
- ✓ Portable across target embedded platforms

### 10.3 Performance Success
- ✓ Execution completes within reasonable time
- ✓ Memory usage within embedded system constraints
- ✓ No unnecessary file system operations
- ✓ Efficient archive creation process

## 11. Migration-Specific Requirements

### 11.1 C Implementation Requirements
- **Standard:** Minimum C99, prefer C11 if available
- **Compiler:** GCC-compatible
- **Memory Management:** Minimize dynamic allocation, prefer stack
- **Error Handling:** Use errno and return codes consistently
- **Portability:** POSIX-compliant system calls

### 11.2 Interface Compatibility
- **Command-Line:** Identical argument structure
- **Log Format:** Maintain compatibility with existing log format
- **File Naming:** Preserve naming conventions
- **Exit Codes:** Compatible with calling scripts

### 11.3 Dependency Migration
- **utils.sh Functions:** Reimplement in C or link to C library
- **uploadSTBLogs.sh:** May remain as shell script (exec from C)
- **System Commands:** Replace with equivalent library calls where possible
  - tar: Consider libarchive
  - Configuration parsing: Implement custom parser

### 11.4 Configuration Parsing
- **Property Files:** Implement parser for key=value format
- **tr181 Integration:** Exec tr181 command or use RBus API if available
- **DCM Configuration:** Support both DCMSettings.conf and dcm.properties

### 11.5 Testing Requirements
- **Unit Tests:** Test each module independently
- **Integration Tests:** Test complete workflow
- **Platform Tests:** Validate on multiple embedded platforms
- **Error Injection:** Test all error handling paths
- **Memory Testing:** Valgrind on development platform

## 12. Non-Functional Requirements

### 12.1 Maintainability
- Modular design with clear separation of concerns
- Well-documented code with comments
- Consistent coding style (follow project standards)
- Easy to extend for new issue types or configuration sources

### 12.2 Reliability
- Robust error handling for all failure modes
- Proper resource cleanup on all exit paths
- Graceful degradation when optional features unavailable
- Idempotent operations where possible

### 12.3 Portability
- POSIX-compliant system calls
- Avoid platform-specific features
- Support for cross-compilation
- Minimal external dependencies

### 12.4 Observability
- Comprehensive logging for debugging
- Clear error messages
- Audit trail of all operations
- Status reporting for monitoring systems

## Document Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | December 1, 2025 | GitHub Copilot | Initial requirements document |
