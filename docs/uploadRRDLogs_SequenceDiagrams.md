# uploadRRDLogs - Sequence Diagrams

## Document Information
- **Component Name:** uploadRRDLogs (C Implementation)
- **Version:** 1.0
- **Date:** December 1, 2025

## 1. Overall System Sequence Diagram

### 1.1 Complete Upload Process (Mermaid)

```mermaid
sequenceDiagram
    actor User
    participant Main as Main Process
    participant Config as Config Manager
    participant SysInfo as System Info Provider
    participant LogProc as Log Processor
    participant Archive as Archive Manager
    participant Upload as Upload Manager
    participant ExtScript as uploadSTBLogs.sh
    participant Log as Logger
    participant FS as File System
    participant Remote as Remote Server

    User->>Main: Execute uploadRRDLogs<br/>UPLOADDIR ISSUETYPE
    activate Main
    
    Main->>Main: Validate Arguments
    alt Invalid Arguments
        Main->>User: Print Usage, Exit 1
    end
    
    Main->>Log: Initialize Logging
    activate Log
    Log->>FS: Open remote-debugger.log
    FS-->>Log: File Handle
    Log-->>Main: Ready
    
    Main->>Config: Load Configuration
    activate Config
    Config->>FS: Read /etc/include.properties
    FS-->>Config: Properties
    Config->>FS: Read /etc/device.properties
    FS-->>Config: Properties
    
    alt Non-prod OR No /opt/dcm.properties
        Config->>ExtScript: Execute tr181 (RFC query)
        activate ExtScript
        ExtScript-->>Config: RFC Parameters
        deactivate ExtScript
        Config->>FS: Read /tmp/DCMSettings.conf
        FS-->>Config: DCM Settings
    else Prod with /opt/dcm.properties
        Config->>FS: Read /opt/dcm.properties
        FS-->>Config: DCM Properties
    end
    
    alt Config incomplete
        Config->>FS: Read /etc/dcm.properties (fallback)
        FS-->>Config: DCM Properties
    end
    
    Config-->>Main: Configuration Data
    deactivate Config
    
    Main->>SysInfo: Get MAC Address
    activate SysInfo
    SysInfo->>FS: Read /sys/class/net/*/address
    FS-->>SysInfo: MAC Address
    SysInfo-->>Main: MAC Address
    
    Main->>SysInfo: Get Timestamp
    SysInfo->>SysInfo: Generate YYYY-MM-DD-HH-MM-SS
    SysInfo-->>Main: Timestamp
    deactivate SysInfo
    
    Main->>LogProc: Validate Source Directory
    activate LogProc
    LogProc->>FS: Check directory exists
    FS-->>LogProc: Exists
    LogProc->>FS: Check directory not empty
    FS-->>LogProc: Has files
    
    alt Empty or Missing
        LogProc->>Log: Log Directory Empty
        LogProc-->>Main: Error
        Main->>User: Exit 0
    end
    
    LogProc->>LogProc: Convert Issue Type to Uppercase
    
    alt Issue Type == LOGUPLOAD_ENABLE
        LogProc->>FS: Check RRD_LIVE_LOGS.tar.gz
        FS-->>LogProc: File exists
        LogProc->>FS: Move to source directory
        FS-->>LogProc: Moved
        LogProc->>Log: Log live logs included
    end
    
    LogProc-->>Main: Source Ready
    deactivate LogProc
    
    Main->>Archive: Create Archive
    activate Archive
    Archive->>Archive: Generate Filename<br/>MAC_ISSUE_TIMESTAMP_RRD_DEBUG_LOGS.tgz
    Archive->>FS: Change to /tmp/rrd/
    FS-->>Archive: Changed
    
    alt Using libarchive
        Archive->>Archive: Initialize libarchive
        Archive->>FS: Open archive file
        FS-->>Archive: File handle
        Archive->>FS: Open source directory
        FS-->>Archive: Directory handle
        
        loop For each file
            Archive->>FS: Read file entry
            FS-->>Archive: File info
            Archive->>FS: Open source file
            FS-->>Archive: File data
            Archive->>Archive: Write to archive (streaming)
            Archive->>FS: Close source file
        end
        
        Archive->>FS: Close archive
        FS-->>Archive: Closed
    else Using tar command
        Archive->>ExtScript: Execute tar -zcf
        activate ExtScript
        ExtScript->>FS: Read source files
        FS-->>ExtScript: File data
        ExtScript->>FS: Write archive
        FS-->>ExtScript: Written
        ExtScript-->>Archive: Exit code 0
        deactivate ExtScript
    end
    
    Archive->>FS: Verify archive exists
    FS-->>Archive: Verified
    Archive->>Log: Log archive created
    Archive-->>Main: Archive Path
    deactivate Archive
    
    Main->>Upload: Upload Archive
    activate Upload
    
    loop Retry up to 10 times
        Upload->>FS: Check /tmp/.log-upload.pid
        FS-->>Upload: Lock status
        
        alt Lock exists
            Upload->>Log: Log waiting for lock
            Upload->>Upload: Sleep 60 seconds
        else Lock free
            Upload->>Upload: Exit retry loop
        end
    end
    
    alt Lock timeout
        Upload->>Log: Log timeout error
        Upload-->>Main: Error
        Main->>FS: Cleanup files
        Main->>User: Exit 4
    end
    
    Upload->>FS: Change to /tmp/rrd/
    FS-->>Upload: Changed
    
    Upload->>ExtScript: Execute uploadSTBLogs.sh<br/>with parameters
    activate ExtScript
    ExtScript->>FS: Create lock file
    ExtScript->>Remote: HTTP/HTTPS POST archive
    activate Remote
    Remote-->>ExtScript: Upload response
    deactivate Remote
    ExtScript->>FS: Remove lock file
    ExtScript-->>Upload: Exit code
    deactivate ExtScript
    
    alt Upload failed
        Upload->>Log: Log upload failure
        Upload->>FS: Remove archive
        Upload->>FS: Remove source directory
        Upload-->>Main: Error
        Main->>User: Exit 4
    else Upload succeeded
        Upload->>Log: Log upload success
        Upload->>FS: Remove archive
        Upload->>FS: Remove source directory
        Upload-->>Main: Success
    end
    
    deactivate Upload
    
    Main->>Log: Close log file
    Log->>FS: Close remote-debugger.log
    deactivate Log
    
    Main->>User: Exit 0
    deactivate Main
```

```
USER                    MAIN             CONFIG          SYSINFO         LOGPROC         ARCHIVE         UPLOAD          EXTSCRIPT       FILESYSTEM      LOGGER
  |                      |                |               |               |               |               |               |               |               |
  |--Execute------------>|                |               |               |               |               |               |               |               |
  |  uploadRRDLogs       |                |               |               |               |               |               |               |               |
  |  UPLOADDIR ISSUETYPE |                |               |               |               |               |               |               |               |
  |                      |                |               |               |               |               |               |               |               |
  |                      |---Validate-----|               |               |               |               |               |               |               |
  |                      |   Arguments    |               |               |               |               |               |               |               |
  |                      |                |               |               |               |               |               |               |               |
  |                      |----------------|---------------|---------------|---------------|---------------|---------------|---------------|-------------->|
  |                      |                                                                                                                    Initialize   |
  |                      |                                                                                                                    Logging      |
  |                      |<----------------------------------------------------------------------------------------------------------------------------------------------|
  |                      |                |               |               |               |               |               |               |               |
  |                      |---Load-------->|               |               |               |               |               |               |               |
  |                      |   Config       |               |               |               |               |               |               |               |
  |                      |                |---------------|---------------|---------------|---------------|---------------|-------------->|               |
  |                      |                |                                                                                   Read properties             |
  |                      |                |<-------------------------------------------------------------------------------|               |               |
  |                      |                |               |               |               |               |               |               |               |
  |                      |                |---------------|---------------|---------------|---------------|-------------->|               |               |
  |                      |                |                                                                   Execute tr181 (if available) |               |
  |                      |                |<------------------------------------------------------------------------------|               |               |
  |                      |                |               |               |               |               |               |               |               |
  |                      |<---Config------|               |               |               |               |               |               |               |
  |                      |    Data        |               |               |               |               |               |               |               |
  |                      |                |               |               |               |               |               |               |               |
  |                      |----------------|-------------->|               |               |               |               |               |               |
  |                      |                                Get MAC Address |               |               |               |               |               |
  |                      |                                |---------------|---------------|---------------|---------------|-------------->|               |
  |                      |                                                                                                    Read MAC     |               |
  |                      |                                |<------------------------------------------------------------------------------|               |
  |                      |<-------------------------------|               |               |               |               |               |               |
  |                      |                                                |               |               |               |               |               |
  |                      |----------------|---------------|-------------->|               |               |               |               |               |
  |                      |                                                Validate Source |               |               |               |               |
  |                      |                                                |---------------|---------------|---------------|-------------->|               |
  |                      |                                                                                                    Check dir    |               |
  |                      |                                                |<------------------------------------------------------------------------------|
  |                      |<-------------------------------|---------------|               |               |               |               |               |
  |                      |                                                                |               |               |               |               |
  |                      |----------------|---------------|---------------|-------------->|               |               |               |               |
  |                      |                                                                Create Archive  |               |               |               |
  |                      |                                                                |---------------|---------------|-------------->|               |
  |                      |                                                                                                    Create tar   |               |
  |                      |                                                                |<------------------------------------------------------------------------------|
  |                      |<-------------------------------|---------------|---------------|               |               |               |               |
  |                      |                                                                                |               |               |               |
  |                      |----------------|---------------|---------------|---------------|-------------->|               |               |               |
  |                      |                                                                                Upload Archive  |               |               |
  |                      |                                                                                |---------------|-------------->|               |
  |                      |                                                                                                    Check lock   |               |
  |                      |                                                                                |<------------------------------------------------------------------------------|
  |                      |                                                                                |               |               |               |
  |                      |                                                                                |---------------|---------------|-------------->|
  |                      |                                                                                                Execute uploadSTBLogs.sh        |
  |                      |                                                                                |<------------------------------------------------------------------------------|
  |                      |<-------------------------------|---------------|---------------|---------------|               |               |               |
  |                      |                                                                                                |               |               |
  |                      |----------------|---------------|---------------|---------------|---------------|---------------|-------------->|               |
  |                      |                                                                                                    Cleanup files                |
  |                      |<----------------------------------------------------------------------------------------------------------------------------------------------|
  |                      |                                                                                                                                |
  |<---Exit 0------------|                                                                                                                                |
```

## 2. Configuration Loading Sequence Diagram

### 2.1 Configuration Loading with Fallbacks (Mermaid)

```mermaid
sequenceDiagram
    participant Main
    participant Config as Config Manager
    participant FS as File System
    participant TR181 as tr181 Tool
    participant Log as Logger

    Main->>Config: rrd_config_load()
    activate Config
    
    Config->>Config: Initialize config struct with defaults
    Config->>Log: Log: Starting configuration load
    
    Config->>FS: Open /etc/include.properties
    activate FS
    FS-->>Config: File content
    deactivate FS
    Config->>Config: Parse key=value pairs<br/>Extract RDK_PATH, LOG_PATH
    
    Config->>FS: Open /etc/device.properties
    activate FS
    FS-->>Config: File content
    deactivate FS
    Config->>Config: Parse key=value pairs<br/>Extract BUILD_TYPE
    
    alt BUILD_TYPE == "prod" AND /opt/dcm.properties exists
        Config->>Log: Log: Production build with override DCM
        Config->>FS: Open /opt/dcm.properties
        activate FS
        FS-->>Config: DCM properties
        deactivate FS
        Config->>Config: Parse and store LOG_SERVER, etc.
        Config->>Log: Log: Skipping RFC (prod override)
    else Non-prod OR no /opt/dcm.properties
        Config->>FS: Check /usr/bin/tr181 exists
        activate FS
        FS-->>Config: Exists: true
        deactivate FS
        
        alt tr181 available
            Config->>Log: Log: Querying RFC parameters
            Config->>TR181: tr181 -g Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.LogServerUrl
            activate TR181
            TR181-->>Config: LOG_SERVER value
            deactivate TR181
            
            Config->>TR181: tr181 -g Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.SsrUrl
            activate TR181
            TR181-->>Config: SsrUrl value
            deactivate TR181
            Config->>Config: Store RFC values
            Config->>Log: Log: RFC parameters retrieved
        else tr181 not available
            Config->>Log: Log: tr181 not available, skipping RFC
        end
        
        Config->>FS: Open /tmp/DCMSettings.conf
        activate FS
        alt File exists
            FS-->>Config: File content
            Config->>Config: Parse LogUploadSettings:UploadRepository:URL
            Config->>Config: Parse LogUploadSettings:UploadRepository:uploadProtocol
            Config->>Log: Log: DCMSettings.conf parsed
        else File not found
            FS-->>Config: Not found
            Config->>Log: Log: DCMSettings.conf not found
        end
        deactivate FS
        
        alt LOG_SERVER or HTTP_UPLOAD_LINK still empty
            Config->>Log: Log: Critical config missing, trying fallback
            Config->>FS: Check /opt/dcm.properties exists
            activate FS
            FS-->>Config: Exists status
            deactivate FS
            
            alt /opt/dcm.properties exists
                Config->>FS: Open /opt/dcm.properties
                activate FS
                FS-->>Config: Properties
                deactivate FS
                Config->>Config: Parse properties
            else Try /etc/dcm.properties
                Config->>FS: Open /etc/dcm.properties
                activate FS
                alt File exists
                    FS-->>Config: Properties
                    Config->>Config: Parse properties
                else File not found
                    FS-->>Config: Not found
                    Config->>Log: Log: ERROR - All config sources failed
                    Config-->>Main: Return ERROR
                end
                deactivate FS
            end
        end
    end
    
    Config->>Config: Validate critical values
    
    alt LOG_SERVER empty
        Config->>Log: Log: ERROR - LOG_SERVER not configured
        Config-->>Main: Return ERROR
    end
    
    alt HTTP_UPLOAD_LINK empty
        Config->>Log: Log: ERROR - HTTP_UPLOAD_LINK not configured
        Config-->>Main: Return ERROR
    end
    
    alt UPLOAD_PROTOCOL empty
        Config->>Config: Set default UPLOAD_PROTOCOL = "HTTP"
        Config->>Log: Log: Using default protocol HTTP
    end
    
    Config->>Log: Log: Configuration loaded successfully<br/>LOG_SERVER, UPLOAD_PROTOCOL, HTTP_UPLOAD_LINK
    Config-->>Main: Return SUCCESS with config data
    deactivate Config
```

## 3. Archive Creation Sequence Diagram

### 3.1 Archive Creation Process (Mermaid)

```mermaid
sequenceDiagram
    participant Main
    participant Archive as Archive Manager
    participant FS as File System
    participant LibArchive as libarchive
    participant TarCmd as tar Command
    participant Log as Logger

    Main->>Archive: rrd_archive_create(source_dir, working_dir, archive_filename)
    activate Archive
    
    Archive->>Log: Log: Starting archive creation
    
    Archive->>Archive: Generate full archive filename<br/>MAC_ISSUE_TIMESTAMP_RRD_DEBUG_LOGS.tgz
    
    Archive->>FS: chdir(/tmp/rrd/)
    activate FS
    FS-->>Archive: Changed
    deactivate FS
    
    Archive->>FS: Check available disk space
    activate FS
    FS-->>Archive: Available space (bytes)
    deactivate FS
    
    alt Insufficient space
        Archive->>Log: Log: ERROR - Insufficient disk space
        Archive-->>Main: Return ERROR_DISK_SPACE
    end
    
    alt libarchive available
        Archive->>Log: Log: Using libarchive for archive creation
        
        Archive->>LibArchive: archive_write_new()
        activate LibArchive
        LibArchive-->>Archive: Archive handle
        
        Archive->>LibArchive: archive_write_add_filter_gzip()
        LibArchive-->>Archive: Success
        
        Archive->>LibArchive: archive_write_set_format_ustar()
        LibArchive-->>Archive: Success
        
        Archive->>LibArchive: archive_write_open_filename(archive_filename)
        LibArchive-->>Archive: Opened
        
        Archive->>FS: opendir(source_dir)
        activate FS
        FS-->>Archive: Directory handle
        
        loop For each file in directory
            Archive->>FS: readdir()
            FS-->>Archive: Directory entry
            
            alt Regular file
                Archive->>FS: stat() to get file info
                FS-->>Archive: File metadata
                
                Archive->>LibArchive: Create entry header
                LibArchive-->>Archive: Entry created
                
                Archive->>LibArchive: archive_write_header()
                LibArchive-->>Archive: Header written
                
                Archive->>FS: open(file)
                FS-->>Archive: File descriptor
                
                loop While data remains
                    Archive->>FS: read(8KB buffer)
                    FS-->>Archive: Data block
                    Archive->>LibArchive: archive_write_data(block)
                    LibArchive-->>Archive: Written
                end
                
                Archive->>FS: close(file)
                FS-->>Archive: Closed
                
                Archive->>LibArchive: archive_write_finish_entry()
                LibArchive-->>Archive: Entry completed
            end
        end
        
        Archive->>FS: closedir()
        FS-->>Archive: Closed
        deactivate FS
        
        Archive->>LibArchive: archive_write_close()
        LibArchive-->>Archive: Closed
        
        Archive->>LibArchive: archive_write_free()
        LibArchive-->>Archive: Freed
        deactivate LibArchive
        
    else libarchive not available
        Archive->>Log: Log: Using tar command for archive creation
        
        Archive->>Archive: Build command string:<br/>tar -zcf archive.tgz -C source_dir .
        
        Archive->>TarCmd: fork() + execl() or system()
        activate TarCmd
        
        TarCmd->>FS: Read source files
        activate FS
        FS-->>TarCmd: File data
        deactivate FS
        
        TarCmd->>FS: Write compressed archive
        activate FS
        FS-->>TarCmd: Written
        deactivate FS
        
        TarCmd-->>Archive: Exit code
        deactivate TarCmd
        
        alt tar exit code != 0
            Archive->>Log: Log: ERROR - tar command failed
            Archive->>FS: Remove partial archive
            Archive-->>Main: Return ERROR_ARCHIVE_FAILED
        end
    end
    
    Archive->>FS: Verify archive file exists
    activate FS
    FS-->>Archive: Exists: true
    deactivate FS
    
    Archive->>FS: Check archive file size > 0
    activate FS
    FS-->>Archive: Size: X bytes
    deactivate FS
    
    alt Archive invalid
        Archive->>Log: Log: ERROR - Archive verification failed
        Archive-->>Main: Return ERROR_ARCHIVE_INVALID
    end
    
    Archive->>Log: Log: Archive created successfully (X bytes)
    Archive-->>Main: Return SUCCESS
    deactivate Archive
```

## 4. Upload Management Sequence Diagram

### 4.1 Upload with Lock Management (Mermaid)

```mermaid
sequenceDiagram
    participant Main
    participant Upload as Upload Manager
    participant FS as File System
    participant Script as uploadSTBLogs.sh
    participant Remote as Remote Server
    participant Log as Logger

    Main->>Upload: rrd_upload_execute(params)
    activate Upload
    
    Upload->>Log: Log: Starting upload process
    
    Upload->>Upload: Initialize: attempt = 1, max_attempts = 10
    
    loop Retry loop (max 10 attempts)
        Upload->>FS: access(/tmp/.log-upload.pid, F_OK)
        activate FS
        FS-->>Upload: Lock status
        deactivate FS
        
        alt Lock exists
            Upload->>Log: Log: Upload lock detected, waiting...
            Upload->>Upload: sleep(60)
            Upload->>Upload: attempt++
            
            alt attempt > max_attempts
                Upload->>Log: Log: ERROR - Lock timeout
                Upload-->>Main: Return ERROR_LOCK_TIMEOUT
            end
        else Lock free
            Upload->>Log: Log: Lock free, proceeding
            Upload->>Upload: Break retry loop
        end
    end
    
    Upload->>FS: chdir(/tmp/rrd/)
    activate FS
    FS-->>Upload: Changed
    deactivate FS
    
    Upload->>Upload: Build command:<br/>uploadSTBLogs.sh LOG_SERVER 1 1 0<br/>PROTOCOL HTTP_LINK 0 1 ARCHIVE
    
    Upload->>Log: Log: Executing upload command
    
    alt Using fork/exec
        Upload->>Upload: fork()
        
        alt Child process
            Upload->>Script: execl(uploadSTBLogs.sh, args)
            activate Script
            
            Script->>FS: Create /tmp/.log-upload.pid
            activate FS
            FS-->>Script: Lock created
            deactivate FS
            
            Script->>Log: Log upload operations
            
            Script->>FS: Read archive file
            activate FS
            FS-->>Script: Archive data
            deactivate FS
            
            Script->>Remote: POST archive via HTTP/HTTPS
            activate Remote
            
            alt Upload successful
                Remote-->>Script: HTTP 200 OK
            else Upload failed
                Remote-->>Script: HTTP error
            end
            deactivate Remote
            
            Script->>FS: Remove /tmp/.log-upload.pid
            activate FS
            FS-->>Script: Lock removed
            deactivate FS
            
            Script-->>Upload: Exit code
            deactivate Script
        end
        
        Upload->>Upload: waitpid(child_pid)
        Upload->>Upload: Get child exit status
        
    else Using system()
        Upload->>Script: system(command_string)
        activate Script
        
        Script->>FS: Create lock, perform upload
        Script->>Remote: Upload archive
        activate Remote
        Remote-->>Script: Response
        deactivate Remote
        Script->>FS: Remove lock
        
        Script-->>Upload: Return code
        deactivate Script
    end
    
    alt Exit code != 0
        Upload->>Log: Log: ERROR - Upload failed (exit code: X)
        Upload-->>Main: Return ERROR_UPLOAD_FAILED
    end
    
    Upload->>Log: Log: Upload completed successfully
    Upload-->>Main: Return SUCCESS
    deactivate Upload
    
    Main->>Upload: rrd_upload_cleanup_files(archive, source_dir)
    activate Upload
    
    Upload->>FS: unlink(archive_path)
    activate FS
    alt Archive removed
        FS-->>Upload: Success
        Upload->>Log: Log: Archive removed
    else Remove failed
        FS-->>Upload: Error
        Upload->>Log: Log: WARNING - Failed to remove archive
    end
    deactivate FS
    
    Upload->>FS: rmdir_recursive(source_dir)
    activate FS
    alt Directory removed
        FS-->>Upload: Success
        Upload->>Log: Log: Source directory removed
    else Remove failed
        FS-->>Upload: Error
        Upload->>Log: Log: WARNING - Failed to remove source
    end
    deactivate FS
    
    Upload->>Log: Log: Cleanup complete
    Upload-->>Main: Return SUCCESS
    deactivate Upload
```

## 5. Error Handling Sequence Diagram

### 5.1 Error Handling Flow (Mermaid)

```mermaid
sequenceDiagram
    participant Module as Any Module
    participant ErrorHandler as Error Handler
    participant Log as Logger
    participant Cleanup as Cleanup Manager
    participant Main as Main Process

    Module->>Module: Detect error condition
    
    Module->>ErrorHandler: Report error with context
    activate ErrorHandler
    
    ErrorHandler->>ErrorHandler: Categorize error<br/>(Fatal/Recoverable/Warning)
    
    alt Fatal Error
        ErrorHandler->>Log: Log FATAL error with full context
        activate Log
        Log-->>ErrorHandler: Logged
        deactivate Log
        
        ErrorHandler->>Cleanup: Initiate cleanup
        activate Cleanup
        
        Cleanup->>Cleanup: Close open files
        Cleanup->>Cleanup: Free allocated memory
        Cleanup->>Cleanup: Remove partial files
        
        Cleanup-->>ErrorHandler: Cleanup complete
        deactivate Cleanup
        
        ErrorHandler->>Main: Return fatal error code (1-3)
        deactivate ErrorHandler
        
        Main->>Main: Exit program with error code
        
    else Recoverable Error
        ErrorHandler->>Log: Log recoverable error
        activate Log
        Log-->>ErrorHandler: Logged
        deactivate Log
        
        ErrorHandler->>ErrorHandler: Check retry count
        
        alt Retry available and not exceeded
            ErrorHandler->>Log: Log retry attempt
            ErrorHandler->>Module: Signal retry
            deactivate ErrorHandler
            Module->>Module: Retry operation
            
        else Retry exhausted or unavailable
            ErrorHandler->>ErrorHandler: Check fallback available
            
            alt Fallback available
                ErrorHandler->>Log: Log using fallback
                ErrorHandler->>Module: Use fallback method
                deactivate ErrorHandler
                Module->>Module: Execute fallback
                
            else No fallback
                ErrorHandler->>Cleanup: Initiate cleanup
                activate Cleanup
                Cleanup->>Cleanup: Cleanup operations
                Cleanup-->>ErrorHandler: Complete
                deactivate Cleanup
                
                ErrorHandler->>Main: Return error code (4)
                deactivate ErrorHandler
                Main->>Main: Exit with error
            end
        end
        
    else Warning
        ErrorHandler->>Log: Log warning
        activate Log
        Log-->>ErrorHandler: Logged
        deactivate Log
        
        ErrorHandler->>ErrorHandler: Set warning flag
        ErrorHandler->>Module: Continue operation
        deactivate ErrorHandler
        Module->>Module: Resume normal flow
    end
```

## Document Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | December 1, 2025 | GitHub Copilot | Initial sequence diagram documentation |
