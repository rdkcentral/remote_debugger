# uploadRRDLogs - Flowchart Documentation

## Document Information
- **Component Name:** uploadRRDLogs (C Implementation)
- **Version:** 1.0
- **Date:** December 1, 2025

## 1. Main Program Flowchart

### 1.1 Overall Program Flow (Mermaid)

```mermaid
flowchart TD
    Start([Start uploadRRDLogs]) --> ValidateArgs{Validate<br/>argc == 3?}
    ValidateArgs -->|No| PrintUsage[Print Usage Message]
    PrintUsage --> Exit1[Exit Code 1]
    
    ValidateArgs -->|Yes| StoreArgs[Store UPLOADDIR<br/>and ISSUETYPE]
    StoreArgs --> InitLog[Initialize Logging<br/>Subsystem]
    InitLog --> LoadConfig[Load Configuration<br/>from Multiple Sources]
    
    LoadConfig --> ConfigOK{Config<br/>Valid?}
    ConfigOK -->|No| LogConfigError[Log Configuration Error]
    LogConfigError --> Exit2[Exit Code 2]
    
    ConfigOK -->|Yes| GetMAC[Get MAC Address<br/>from System]
    GetMAC --> MACOK{MAC<br/>Retrieved?}
    MACOK -->|No| LogMACError[Log MAC Error]
    LogMACError --> Exit2
    
    MACOK -->|Yes| GetTimestamp[Generate Timestamp<br/>YYYY-MM-DD-HH-MM-SS]
    GetTimestamp --> ValidateDir{Source Dir<br/>Exists and<br/>Not Empty?}
    
    ValidateDir -->|No| LogDirError[Log Directory Empty/Missing]
    LogDirError --> Exit3[Exit Code 0]
    
    ValidateDir -->|Yes| ConvertIssue[Convert ISSUETYPE<br/>to Uppercase]
    ConvertIssue --> CheckSpecial{Issue Type ==<br/>LOGUPLOAD_ENABLE?}
    
    CheckSpecial -->|Yes| MoveLiveLogs[Move RRD_LIVE_LOGS.tar.gz<br/>to Source Directory]
    MoveLiveLogs --> GenFilename
    CheckSpecial -->|No| GenFilename[Generate Archive Filename<br/>MAC_ISSUE_TIME_RRD_DEBUG_LOGS.tgz]
    
    GenFilename --> CreateArchive[Create tar.gz Archive<br/>from Source Directory]
    CreateArchive --> ArchiveOK{Archive<br/>Created?}
    
    ArchiveOK -->|No| LogArchiveError[Log Archive Error]
    LogArchiveError --> Cleanup1[Cleanup Partial Files]
    Cleanup1 --> Exit3B[Exit Code 3]
    
    ArchiveOK -->|Yes| CheckLock{Check Upload<br/>Lock File<br/>/tmp/.log-upload.pid}
    
    CheckLock -->|Exists| WaitLock[Wait 60 seconds]
    WaitLock --> IncAttempt[Increment Attempt Counter]
    IncAttempt --> MaxAttempts{Attempts<br/> <= 10?}
    MaxAttempts -->|Yes| CheckLock
    MaxAttempts -->|No| LogLockTimeout[Log Lock Timeout Error]
    LogLockTimeout --> Cleanup2[Remove Archive and Source]
    Cleanup2 --> Exit4[Exit Code 4]
    
    CheckLock -->|Not Exists| InvokeUpload[Invoke uploadSTBLogs.sh<br/>with Parameters]
    InvokeUpload --> UploadOK{Upload<br/>Success?}
    
    UploadOK -->|No| LogUploadFail[Log Upload Failure]
    LogUploadFail --> Cleanup3[Remove Archive and Source]
    Cleanup3 --> Exit4B[Exit Code 4]
    
    UploadOK -->|Yes| LogUploadSuccess[Log Upload Success]
    LogUploadSuccess --> Cleanup4[Remove Archive and Source]
    Cleanup4 --> CleanupOK{Cleanup<br/>Success?}
    
    CleanupOK -->|No| LogCleanupWarn[Log Cleanup Warning]
    LogCleanupWarn --> Exit0[Exit Code 0]
    CleanupOK -->|Yes| Exit0
    
    Exit1 --> End([End])
    Exit2 --> End
    Exit3 --> End
    Exit3B --> End
    Exit4 --> End
    Exit4B --> End
    Exit0 --> End
```

### 1.2 Simplified Text-Based Main Flow

```
START
  |
  v
[Validate Command-Line Arguments]
  |
  ├─> (Invalid) --> [Print Usage] --> EXIT(1)
  |
  v (Valid)
[Initialize Logging]
  |
  v
[Load Configuration]
  |
  ├─> (Failed) --> [Log Error] --> EXIT(2)
  |
  v (Success)
[Get System Information (MAC, Timestamp)]
  |
  ├─> (Failed) --> [Log Error] --> EXIT(2)
  |
  v (Success)
[Validate Source Directory]
  |
  ├─> (Empty/Missing) --> [Log Message] --> EXIT(0)
  |
  v (Valid)
[Process Issue Type (Uppercase, Special Handling)]
  |
  v
[Generate Archive Filename]
  |
  v
[Create tar.gz Archive]
  |
  ├─> (Failed) --> [Cleanup] --> EXIT(3)
  |
  v (Success)
[Check and Wait for Upload Lock]
  |
  ├─> (Timeout) --> [Cleanup] --> EXIT(4)
  |
  v (Lock Free)
[Execute Upload Script]
  |
  ├─> (Failed) --> [Cleanup] --> EXIT(4)
  |
  v (Success)
[Cleanup Archive and Source]
  |
  v
EXIT(0)
  |
  v
END
```

## 2. Configuration Loading Flowchart

### 2.1 Configuration Loading (Mermaid)

```mermaid
flowchart TD
    Start([Start Config Loading]) --> InitStruct[Initialize Config Structure<br/>with Defaults]
    InitStruct --> LoadInclude[Load /etc/include.properties]
    LoadInclude --> IncludeOK{File<br/>Loaded?}
    
    IncludeOK -->|No| LogIncludeWarn[Log Warning]
    IncludeOK -->|Yes| ParseInclude[Parse Properties]
    LogIncludeWarn --> LoadDevice
    ParseInclude --> LoadDevice[Load /etc/device.properties]
    
    LoadDevice --> DeviceOK{File<br/>Loaded?}
    DeviceOK -->|No| LogDeviceWarn[Log Warning]
    DeviceOK -->|Yes| ParseDevice[Parse Properties]
    LogDeviceWarn --> CheckBuildType
    ParseDevice --> CheckBuildType{BUILD_TYPE == prod<br/>AND /opt/dcm.properties<br/>exists?}
    
    CheckBuildType -->|Yes| LoadOptDCM[Load /opt/dcm.properties<br/>OVERRIDE RFC]
    LoadOptDCM --> ParseOptDCM[Parse DCM Properties]
    ParseOptDCM --> ValidateConfig
    
    CheckBuildType -->|No| CheckTR181{/usr/bin/tr181<br/>exists?}
    CheckTR181 -->|No| SkipRFC[Skip RFC Query]
    SkipRFC --> LoadDCMSettings
    
    CheckTR181 -->|Yes| QueryLogServer[Query RFC:<br/>LogServerUrl]
    QueryLogServer --> QuerySSR[Query RFC:<br/>SsrUrl]
    QuerySSR --> ParseRFC[Parse RFC Results]
    ParseRFC --> LoadDCMSettings[Load /tmp/DCMSettings.conf]
    
    LoadDCMSettings --> DCMSettingsOK{File<br/>Exists?}
    DCMSettingsOK -->|No| LoadFallbackDCM
    DCMSettingsOK -->|Yes| ParseDCMSettings[Parse DCM Settings:<br/>- UploadRepository:URL<br/>- uploadProtocol]
    ParseDCMSettings --> ValidateConfig
    
    DCMSettingsOK -->|No| LoadFallbackDCM[Load dcm.properties<br/>/opt or /etc]
    LoadFallbackDCM --> FallbackOK{File<br/>Loaded?}
    FallbackOK -->|No| LogFallbackError[Log Error]
    LogFallbackError --> ReturnError[Return Error]
    FallbackOK -->|Yes| ParseFallbackDCM[Parse Fallback DCM]
    ParseFallbackDCM --> ValidateConfig
    
    ValidateConfig{LOG_SERVER and<br/>HTTP_UPLOAD_LINK<br/>not empty?}
    ValidateConfig -->|No| SetDefaults[Set Default Protocol<br/>HTTP if missing]
    SetDefaults --> StillInvalid{Required<br/>Values<br/>Missing?}
    StillInvalid -->|Yes| ReturnError
    StillInvalid -->|No| LogConfig
    
    ValidateConfig -->|Yes| LogConfig[Log Configuration Summary]
    LogConfig --> ReturnSuccess[Return Success]
    
    ReturnError --> End([End])
    ReturnSuccess --> End
```

### 2.2 Text-Based Configuration Flow

```
START Configuration Loading
  |
  v
[Initialize config structure with defaults]
  |
  v
[Load /etc/include.properties]
  ├─> Parse key=value pairs
  ├─> Extract RDK_PATH, LOG_PATH, etc.
  |
  v
[Load /etc/device.properties]
  ├─> Parse key=value pairs
  ├─> Extract BUILD_TYPE, etc.
  |
  v
{Check: BUILD_TYPE == "prod" AND /opt/dcm.properties exists?}
  |
  ├─> YES: [Load /opt/dcm.properties] --> [Skip RFC] --> VALIDATE
  |
  └─> NO: Continue
      |
      v
    {Check: /usr/bin/tr181 exists?}
      |
      ├─> NO: [Skip RFC Query] --> [Load DCMSettings.conf]
      |
      └─> YES: [Query RFC Parameters]
          |
          ├─> tr181 -g Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.LogServerUrl
          ├─> tr181 -g Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.SsrUrl
          ├─> Store results
          |
          v
        [Load /tmp/DCMSettings.conf]
          |
          ├─> Parse: LogUploadSettings:UploadRepository:URL
          ├─> Parse: LogUploadSettings:UploadRepository:uploadProtocol
          |
          v
        {DCMSettings.conf loaded successfully?}
          |
          ├─> YES: VALIDATE
          |
          └─> NO: [Load /opt/dcm.properties or /etc/dcm.properties]
              |
              ├─> Parse properties
              |
              v
            {Fallback loaded?}
              |
              ├─> NO: RETURN ERROR
              |
              └─> YES: VALIDATE
                  |
                  v
                VALIDATE:
                  |
                  v
                {LOG_SERVER not empty?}
                  |
                  ├─> NO: RETURN ERROR
                  |
                  v
                {HTTP_UPLOAD_LINK not empty?}
                  |
                  ├─> NO: RETURN ERROR
                  |
                  v
                {UPLOAD_PROTOCOL empty?}
                  |
                  ├─> YES: Set default "HTTP"
                  |
                  v
                [Log configuration summary]
                  |
                  v
                RETURN SUCCESS
```

## 3. Archive Creation Flowchart

### 3.1 Archive Creation (Mermaid)

```mermaid
flowchart TD
    Start([Start Archive Creation]) --> ChangeDir[Change to Working Directory<br/>/tmp/rrd/]
    ChangeDir --> DirOK{Directory<br/>Accessible?}
    
    DirOK -->|No| LogDirError[Log Directory Error]
    LogDirError --> ReturnError[Return Error Code 3]
    
    DirOK -->|Yes| GenFilename[Generate Archive Filename<br/>MAC_ISSUE_TIMESTAMP_RRD_DEBUG_LOGS.tgz]
    GenFilename --> CheckSpace{Check<br/>Disk Space<br/>Available?}
    
    CheckSpace -->|No| LogSpaceError[Log Disk Space Error]
    LogSpaceError --> ReturnError
    
    CheckSpace -->|Yes| CheckLibarchive{libarchive<br/>available?}
    
    CheckLibarchive -->|Yes| UseLibarchive[Use libarchive API]
    UseLibarchive --> InitArchive[archive_write_new]
    InitArchive --> SetGzip[archive_write_add_filter_gzip]
    SetGzip --> SetFormat[archive_write_set_format_ustar]
    SetFormat --> OpenArchive[archive_write_open_filename]
    OpenArchive --> OpenOK{Open<br/>Success?}
    
    OpenOK -->|No| LogOpenError[Log Open Error]
    LogOpenError --> ReturnError
    
    OpenOK -->|Yes| OpenSourceDir[Open Source Directory]
    OpenSourceDir --> ReadEntry[Read Directory Entry]
    ReadEntry --> MoreEntries{More<br/>Entries?}
    
    MoreEntries -->|No| CloseArchive[archive_write_close]
    CloseArchive --> VerifyArchive
    
    MoreEntries -->|Yes| IsFile{Is Regular<br/>File?}
    IsFile -->|No| ReadEntry
    
    IsFile -->|Yes| CreateHeader[Create Archive Entry Header]
    CreateHeader --> WriteHeader[archive_write_header]
    WriteHeader --> OpenFile[Open Source File]
    OpenFile --> FileOK{File<br/>Opened?}
    
    FileOK -->|No| LogFileWarn[Log Warning]
    LogFileWarn --> ReadEntry
    
    FileOK -->|Yes| ReadBlock[Read File Block<br/>8KB buffer]
    ReadBlock --> BlockData{Data<br/>Read?}
    
    BlockData -->|Yes| WriteBlock[archive_write_data]
    WriteBlock --> ReadBlock
    
    BlockData -->|No| CloseFile[Close Source File]
    CloseFile --> FinishEntry[archive_write_finish_entry]
    FinishEntry --> ReadEntry
    
    CheckLibarchive -->|No| UseTarCommand[Use tar Command]
    UseTarCommand --> BuildCmd[Build Command:<br/>tar -zcf archive.tgz -C source_dir .]
    BuildCmd --> RedirectStderr[Redirect stderr to log]
    RedirectStderr --> ExecTar[Execute tar via system/fork/exec]
    ExecTar --> TarResult{tar Exit<br/>Code == 0?}
    
    TarResult -->|No| LogTarError[Log tar Error]
    LogTarError --> RemovePartial[Remove Partial Archive]
    RemovePartial --> ReturnError
    
    TarResult -->|Yes| VerifyArchive{Archive File<br/>Exists and<br/>Size > 0?}
    
    VerifyArchive -->|No| LogVerifyError[Log Verification Error]
    LogVerifyError --> ReturnError
    
    VerifyArchive -->|Yes| LogSuccess[Log Archive Created]
    LogSuccess --> ReturnSuccess[Return Success]
    
    ReturnError --> End([End])
    ReturnSuccess --> End
```

### 3.2 Text-Based Archive Creation Flow

```
START Archive Creation
  |
  v
[Change to working directory: /tmp/rrd/]
  |
  ├─> (Failed) --> [Log Error] --> RETURN ERROR(3)
  |
  v (Success)
[Generate archive filename from MAC, Issue Type, Timestamp]
  |
  v
[Check available disk space in /tmp]
  |
  ├─> (Insufficient) --> [Log Error] --> RETURN ERROR(3)
  |
  v (Sufficient)
{Check: libarchive available?}
  |
  ├─> YES: [Use Native C Archive Creation]
  |     |
  |     v
  |   [Initialize libarchive]
  |   [Set gzip compression]
  |   [Set tar format (ustar)]
  |   [Open output archive file]
  |     |
  |     ├─> (Open Failed) --> [Log Error] --> RETURN ERROR(3)
  |     |
  |     v (Opened)
  |   [Open source directory]
  |   FOR each file in directory:
  |     |
  |     ├─> [Create archive entry header]
  |     ├─> [Write header to archive]
  |     ├─> [Open source file]
  |     |     |
  |     |     ├─> (Failed) --> [Log Warning] --> Continue
  |     |     |
  |     |     v (Opened)
  |     |   WHILE data remaining:
  |     |     |
  |     |     ├─> [Read 8KB block from file]
  |     |     ├─> [Write block to archive]
  |     |     └─> Loop
  |     |   END WHILE
  |     |     |
  |     |     v
  |     ├─> [Close source file]
  |     ├─> [Finish archive entry]
  |     └─> Next file
  |   END FOR
  |     |
  |     v
  |   [Close archive]
  |     |
  |     v
  |   GOTO VERIFY
  |
  └─> NO: [Use tar Command Method]
        |
        v
      [Build command: tar -zcf archive.tgz -C source_dir .]
      [Redirect stderr to log file]
      [Execute tar via fork/exec or system]
        |
        v
      {tar exit code == 0?}
        |
        ├─> NO: [Log tar error] --> [Remove partial files] --> RETURN ERROR(3)
        |
        v YES
      VERIFY:
        |
        v
      [Check archive file exists]
      [Check archive file size > 0]
        |
        ├─> (Invalid) --> [Log Error] --> RETURN ERROR(3)
        |
        v (Valid)
      [Log success message]
        |
        v
      RETURN SUCCESS
        |
        v
      END
```

## 4. Upload Management Flowchart

### 4.1 Upload with Lock Management (Mermaid)

```mermaid
flowchart TD
    Start([Start Upload Process]) --> InitVars[Initialize Variables:<br/>attempt = 1<br/>max_attempts = 10<br/>wait_seconds = 60]
    InitVars --> CheckLock{Check Lock File<br/>/tmp/.log-upload.pid<br/>exists?}
    
    CheckLock -->|Not Exists| LogProceed[Log: Lock Free, Proceeding]
    LogProceed --> ChangeDir[Change to Working Dir<br/>/tmp/rrd/]
    
    CheckLock -->|Exists| LogWait[Log: Upload Lock Detected<br/>Waiting 60 seconds]
    LogWait --> Sleep[Sleep 60 seconds]
    Sleep --> IncAttempt[Increment attempt]
    IncAttempt --> CheckMax{attempt <=<br/>max_attempts?}
    
    CheckMax -->|No| LogTimeout[Log: Lock Timeout Error]
    LogTimeout --> ReturnLockError[Return Error Code 4]
    
    CheckMax -->|Yes| CheckLock
    
    ChangeDir --> DirOK{Directory<br/>Change OK?}
    DirOK -->|No| LogDirError[Log Directory Error]
    LogDirError --> ReturnError[Return Error Code 4]
    
    DirOK -->|Yes| BuildCmd[Build uploadSTBLogs.sh Command:<br/>- LOG_SERVER<br/>- Flags: 1 1 0<br/>- UPLOAD_PROTOCOL<br/>- HTTP_UPLOAD_LINK<br/>- Flags: 0 1<br/>- ARCHIVE_FILENAME]
    
    BuildCmd --> LogCmd[Log Command to be Executed]
    LogCmd --> CheckExec{Choose<br/>Execution<br/>Method}
    
    CheckExec -->|fork/exec| Fork[fork process]
    Fork --> ForkOK{Fork<br/>Success?}
    
    ForkOK -->|No| LogForkError[Log Fork Error]
    LogForkError --> ReturnError
    
    ForkOK -->|Yes| IsChild{Child<br/>Process?}
    IsChild -->|Yes| ExecScript[execl uploadSTBLogs.sh<br/>with arguments]
    ExecScript --> ExecFailed[Log Exec Failed<br/>If reached]
    ExecFailed --> ExitChild[_exit 127]
    
    IsChild -->|No| WaitChild[waitpid for child]
    WaitChild --> CheckStatus{Child Exit<br/>Status == 0?}
    
    CheckExec -->|system| SystemCall[system uploadSTBLogs.sh<br/>with arguments]
    SystemCall --> SystemResult{Return<br/>Code == 0?}
    
    SystemResult -->|No| LogSystemError[Log Upload Failed]
    LogSystemError --> ReturnError
    
    CheckStatus -->|No| LogUploadFail[Log Upload Failed]
    LogUploadFail --> ReturnError
    
    SystemResult -->|Yes| LogUploadSuccess
    CheckStatus -->|Yes| LogUploadSuccess[Log Upload Success]
    LogUploadSuccess --> ReturnSuccess[Return Success Code 0]
    
    ReturnLockError --> End([End])
    ReturnError --> End
    ExitChild --> End
    ReturnSuccess --> End
```

### 4.2 Text-Based Upload Flow

```
START Upload Process
  |
  v
[Initialize: attempt=1, max_attempts=10, wait_seconds=60]
  |
  v
RETRY_LOOP:
  |
  v
{Check: /tmp/.log-upload.pid exists?}
  |
  ├─> NO: [Lock is free] --> PROCEED_UPLOAD
  |
  └─> YES: [Lock is held]
      |
      v
    [Log: Another upload in progress, waiting...]
      |
      v
    [Sleep 60 seconds]
      |
      v
    [Increment attempt counter]
      |
      v
    {attempt <= 10?}
      |
      ├─> NO: [Log: Lock timeout error]
      |       |
      |       v
      |     RETURN ERROR(4)
      |
      └─> YES: GOTO RETRY_LOOP

PROCEED_UPLOAD:
  |
  v
[Change directory to /tmp/rrd/]
  |
  ├─> (Failed) --> [Log Error] --> RETURN ERROR(4)
  |
  v (Success)
[Build uploadSTBLogs.sh command with parameters:
   - $RDK_PATH/uploadSTBLogs.sh
   - LOG_SERVER
   - 1
   - 1
   - 0
   - UPLOAD_PROTOCOL
   - HTTP_UPLOAD_LINK
   - 0
   - 1
   - ARCHIVE_FILENAME]
  |
  v
[Log command being executed]
  |
  v
{Choose execution method: fork/exec or system?}
  |
  ├─> fork/exec:
  |     |
  |     v
  |   [Fork new process]
  |     |
  |     ├─> (Fork Failed) --> [Log Error] --> RETURN ERROR(4)
  |     |
  |     v (Fork Success)
  |   {Am I child process?}
  |     |
  |     ├─> YES: [execl uploadSTBLogs.sh with args]
  |     |       |
  |     |       └─> (If execl returns) [Log Error] [_exit(127)]
  |     |
  |     └─> NO: [Parent: waitpid for child]
  |           |
  |           v
  |         [Get child exit status]
  |           |
  |           v
  |         {Exit status == 0?}
  |           |
  |           ├─> NO: [Log Upload Failed] --> RETURN ERROR(4)
  |           |
  |           └─> YES: UPLOAD_SUCCESS
  |
  └─> system:
        |
        v
      [system(full_command_string)]
        |
        v
      [Get return code]
        |
        v
      {Return code == 0?}
        |
        ├─> NO: [Log Upload Failed] --> RETURN ERROR(4)
        |
        └─> YES: UPLOAD_SUCCESS

UPLOAD_SUCCESS:
  |
  v
[Log: Upload completed successfully]
  |
  v
RETURN SUCCESS(0)
  |
  v
END
```

## 5. Cleanup Operations Flowchart

### 5.1 Cleanup Process (Mermaid)

```mermaid
flowchart TD
    Start([Start Cleanup]) --> InputParams[Input Parameters:<br/>- archive_path<br/>- source_dir<br/>- upload_status]
    
    InputParams --> LogStart[Log: Starting Cleanup]
    LogStart --> RemoveArchive[Remove Archive File]
    RemoveArchive --> ArchiveRemoved{Archive<br/>Removed?}
    
    ArchiveRemoved -->|No| CheckArchiveExists{Archive<br/>Still Exists?}
    CheckArchiveExists -->|Yes| LogArchiveError[Log: Failed to Remove Archive]
    CheckArchiveExists -->|No| LogArchiveNotFound[Log: Archive Already Removed]
    LogArchiveError --> RemoveSource
    LogArchiveNotFound --> RemoveSource
    
    ArchiveRemoved -->|Yes| LogArchiveSuccess[Log: Archive Removed]
    LogArchiveSuccess --> RemoveSource[Remove Source Directory<br/>Recursively]
    
    RemoveSource --> SourceRemoved{Source Dir<br/>Removed?}
    SourceRemoved -->|No| CheckSourceExists{Source<br/>Still Exists?}
    CheckSourceExists -->|Yes| LogSourceError[Log: Failed to Remove Source]
    CheckSourceExists -->|No| LogSourceNotFound[Log: Source Already Removed]
    LogSourceError --> DetermineResult
    LogSourceNotFound --> DetermineResult
    
    SourceRemoved -->|Yes| LogSourceSuccess[Log: Source Directory Removed]
    LogSourceSuccess --> DetermineResult{Upload<br/>Was Successful?}
    
    DetermineResult -->|Yes| LogCleanupComplete[Log: Cleanup Complete - Upload Success]
    LogCleanupComplete --> ReturnSuccess[Return Success]
    
    DetermineResult -->|No| LogCleanupFailed[Log: Cleanup Complete - Upload Failed]
    LogCleanupFailed --> ReturnError[Return Error]
    
    ReturnSuccess --> End([End])
    ReturnError --> End
```

### 5.2 Text-Based Cleanup Flow

```
START Cleanup
  |
  v
INPUT: archive_path, source_dir, upload_status
  |
  v
[Log: Starting cleanup operations]
  |
  v
[Attempt to remove archive file: archive_path]
  |
  v
{Archive removed successfully?}
  |
  ├─> NO: {Archive file still exists?}
  |       |
  |       ├─> YES: [Log: WARNING - Failed to remove archive]
  |       |
  |       └─> NO: [Log: INFO - Archive not found (already removed?)]
  |
  └─> YES: [Log: INFO - Archive removed successfully]
  |
  v
[Attempt to remove source directory recursively: source_dir]
  |
  v
{Source directory removed successfully?}
  |
  ├─> NO: {Source directory still exists?}
  |       |
  |       ├─> YES: [Log: WARNING - Failed to remove source directory]
  |       |
  |       └─> NO: [Log: INFO - Source not found (already removed?)]
  |
  └─> YES: [Log: INFO - Source directory removed successfully]
  |
  v
{Was upload operation successful?}
  |
  ├─> YES: [Log: Cleanup complete - Upload succeeded]
  |         |
  |         v
  |       RETURN SUCCESS(0)
  |
  └─> NO: [Log: Cleanup complete - Upload failed]
          |
          v
        RETURN ERROR(4)
  |
  v
END
```

## 6. Special Case: LOGUPLOAD_ENABLE Flowchart

### 6.1 LOGUPLOAD_ENABLE Handling (Mermaid)

```mermaid
flowchart TD
    Start([Check Issue Type]) --> CompareIssue{Issue Type ==<br/>LOGUPLOAD_ENABLE?}
    
    CompareIssue -->|No| SkipSpecial[Skip Special Handling]
    SkipSpecial --> ContinueNormal[Continue Normal Flow]
    
    CompareIssue -->|Yes| LogSpecial[Log: Handling LOGUPLOAD_ENABLE]
    LogSpecial --> CheckLiveLog{RRD_LIVE_LOGS.tar.gz<br/>exists in /tmp/rrd/?}
    
    CheckLiveLog -->|No| LogNoLive[Log: Live logs not found]
    LogNoLive --> ContinueNormal
    
    CheckLiveLog -->|Yes| LogFoundLive[Log: Found live logs file]
    LogFoundLive --> MoveLive[Move RRD_LIVE_LOGS.tar.gz<br/>to source directory]
    MoveLive --> MoveOK{Move<br/>Success?}
    
    MoveOK -->|No| LogMoveError[Log: Warning - Failed to move live logs]
    LogMoveError --> ContinueNormal
    
    MoveOK -->|Yes| LogMoveSuccess[Log: Live logs moved successfully]
    LogMoveSuccess --> ContinueNormal
    
    ContinueNormal --> End([Continue to Archive Creation])
```

### 6.2 Text-Based LOGUPLOAD_ENABLE Flow

```
START Issue Type Processing
  |
  v
[Convert issue_type to uppercase]
  |
  v
{issue_type == "LOGUPLOAD_ENABLE"?}
  |
  ├─> NO: CONTINUE_NORMAL
  |
  └─> YES: [Handle special case]
      |
      v
    [Log: Check for live device logs]
      |
      v
    {Check: /tmp/rrd/RRD_LIVE_LOGS.tar.gz exists?}
      |
      ├─> NO: [Log: Live logs file not found]
      |       |
      |       v
      |     CONTINUE_NORMAL
      |
      └─> YES: [Log: Live logs file found]
          |
          v
        [Attempt to move RRD_LIVE_LOGS.tar.gz to source_dir]
          |
          v
        {Move successful?}
          |
          ├─> NO: [Log: WARNING - Failed to move live logs]
          |       [Log: Continuing without live logs]
          |       |
          |       v
          |     CONTINUE_NORMAL
          |
          └─> YES: [Log: Live logs included in upload]
              |
              v
            CONTINUE_NORMAL

CONTINUE_NORMAL:
  |
  v
[Proceed to archive creation]
  |
  v
END
```

## 7. Error Handling Decision Tree

### 7.1 Error Handling Flow (Mermaid)

```mermaid
flowchart TD
    Start([Error Detected]) --> Categorize{Error<br/>Category?}
    
    Categorize -->|Fatal| LogFatal[Log: FATAL ERROR with context]
    LogFatal --> CleanupFatal[Cleanup Resources]
    CleanupFatal --> ExitFatal[Exit with Error Code 1-3]
    
    Categorize -->|Recoverable| LogRecover[Log: Recoverable Error]
    LogRecover --> CheckRetry{Retry<br/>Available?}
    CheckRetry -->|Yes| IncrementRetry[Increment Retry Counter]
    IncrementRetry --> CheckMaxRetry{Max Retries<br/>Exceeded?}
    CheckMaxRetry -->|No| RetryOperation[Retry Operation]
    RetryOperation --> End1([Return to Operation])
    CheckMaxRetry -->|Yes| LogMaxRetry[Log: Max Retries Exceeded]
    LogMaxRetry --> CleanupRecover[Cleanup Resources]
    CleanupRecover --> ExitRecover[Exit with Error Code 4]
    CheckRetry -->|No| TryFallback{Fallback<br/>Available?}
    TryFallback -->|Yes| UseFallback[Use Fallback Method]
    UseFallback --> End2([Return to Operation])
    TryFallback -->|No| CleanupRecover
    
    Categorize -->|Warning| LogWarning[Log: WARNING with context]
    LogWarning --> MarkWarning[Set Warning Flag]
    MarkWarning --> ContinueWarn[Continue Operation]
    ContinueWarn --> End3([Return to Operation])
    
    ExitFatal --> End([Program Termination])
    ExitRecover --> End
```

### 7.2 Text-Based Error Handling

```
ERROR DETECTED
  |
  v
DETERMINE ERROR CATEGORY:
  |
  ├─> FATAL ERROR (Cannot Continue)
  |     |
  |     v
  |   [Log error with full context: timestamp, operation, error details]
  |     |
  |     v
  |   [Perform cleanup: close files, free memory, remove partial files]
  |     |
  |     v
  |   [Exit with appropriate code:
  |      1 = Argument error
  |      2 = Configuration error
  |      3 = Archive error]
  |     |
  |     v
  |   PROGRAM TERMINATION
  |
  ├─> RECOVERABLE ERROR (Can Retry)
  |     |
  |     v
  |   [Log error with context]
  |     |
  |     v
  |   {Retry mechanism available?}
  |     |
  |     ├─> YES: {Max retries exceeded?}
  |     |       |
  |     |       ├─> NO: [Wait/backoff] --> [Retry operation]
  |     |       |
  |     |       └─> YES: [Cleanup] --> [Exit with code 4]
  |     |
  |     └─> NO: {Fallback method available?}
  |           |
  |           ├─> YES: [Use fallback] --> CONTINUE
  |           |
  |           └─> NO: [Cleanup] --> [Exit with code 4]
  |
  └─> WARNING (Non-Critical)
        |
        v
      [Log warning message]
        |
        v
      [Set warning flag (optional)]
        |
        v
      [Continue normal operation]
        |
        v
      RETURN TO OPERATION
```

## Document Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | December 1, 2025 | GitHub Copilot | Initial flowchart documentation |
