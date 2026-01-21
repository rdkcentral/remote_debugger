##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2018 RDK Management
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

from helper_functions import *

def test_check_remote_debugger_config_file():
    config_file_path = JSON_FILE
    assert check_file_exists(config_file_path), f"Configuration file '{config_file_path}' does not exist."

def test_check_rrd_directory_exists():
    dir_path = OUTPUT_DIR
    assert check_directory_exists(dir_path), f"Directory '{dir_path}' does not exist."

def reset_issuetype_rfc():
    command = 'rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType string ""'
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    assert result.returncode == 0

def test_check_and_start_remotedebugger():
    kill_rrd()
    remove_logfile()
    print("Starting remotedebugger process")
    command_to_start = "nohup /usr/local/bin/remotedebugger > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    command_to_get_pid = "pidof remotedebugger"
    pid = run_shell_command(command_to_get_pid)
    assert pid != "", "remotedebugger process did not start"

def test_remote_debugger_trigger_event():
    reset_issuetype_rfc()
    sleep(10)
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', ISSUE_STRING
    ]
    result = subprocess.run(command, capture_output=True, text=True)
    assert result.returncode == 0, f"Command failed with error: {result.stderr}"

    sleep(15)

    QUERY_MSG = "Received event for RRD_SET_ISSUE_EVENT"
    assert QUERY_MSG in grep_rrdlogs(QUERY_MSG)

    MSG_SEND = "SUCCESS: Message sending Done"
    sleep(2)
    assert MSG_SEND in grep_rrdlogs(MSG_SEND)

    MSG_RECEIVE = "SUCCESS: Message Reception Done"
    sleep(2)
    assert MSG_RECEIVE in grep_rrdlogs(MSG_RECEIVE)

    ISSUE_MSG = f'MSG={ISSUE_STRING}'
    assert ISSUE_MSG in grep_rrdlogs(ISSUE_MSG)
    print("Sent and received messages are found and match in the logfile")

    READ_JSON = "Start Reading JSON File... /etc/rrd/remote_debugger.json"
    assert READ_JSON in grep_rrdlogs(READ_JSON)

    PARSE_JSON = "Json File parse Success... /etc/rrd/remote_debugger.json"
    assert PARSE_JSON in grep_rrdlogs(PARSE_JSON)

    if '.' in ISSUE_STRING:
        ISSUE_NODE, ISSUE_SUBNODE = ISSUE_STRING.split('.')
    else:
        node = ISSUE_STRING
        subnode = None

    ISSUE_FOUND = f"Issue Data Node: {ISSUE_NODE} and Sub-Node: {ISSUE_SUBNODE} found in Static JSON File /etc/rrd/remote_debugger.json"

    DIR_CREATION = "Creating Directory"
    assert DIR_CREATION in grep_rrdlogs(DIR_CREATION)

    SANITY_CHECK = "Found valid Commands"
    assert SANITY_CHECK in grep_rrdlogs(SANITY_CHECK)

    DEBUG_FILE = "Adding Details of Debug commands to Output File"
    assert DEBUG_FILE in grep_rrdlogs(DEBUG_FILE)

    SERVICE_START = f"Starting remote_debugger_{ISSUE_STRING} service success"
    assert SERVICE_START in grep_rrdlogs(SERVICE_START)

    JOURNAL_START = f"journalctl remote_debugger_{ISSUE_STRING} service success"
    assert JOURNAL_START in grep_rrdlogs(JOURNAL_START)

    SLEEP_TIME = "Sleeping with timeout"
    assert SLEEP_TIME in grep_rrdlogs(SLEEP_TIME)
    sleep(20)

    SERVICE_STOP = f"Stopping remote_debugger_{ISSUE_STRING} service"
    assert SERVICE_STOP in grep_rrdlogs(SERVICE_STOP)

    result = check_output_dir()
    print(result)

    UPLOAD_LOGS = "Starting Upload Debug output via API"
    assert UPLOAD_LOGS in grep_rrdlogs(UPLOAD_LOGS)

    # Verify rrd_upload_orchestrate() function logs
    print("Validating rrd_upload_orchestrate logs...")
    
    # Check orchestration entry
    ORCHESTRATE_ENTRY = "Executing binary to upload Debug info of ISSUETYPE"
    assert ORCHESTRATE_ENTRY in grep_rrdlogs(ORCHESTRATE_ENTRY), "Missing orchestration entry log"
    
    # Check logging subsystem initialization
    LOGGING_READY = "Logging ready"
    assert LOGGING_READY in grep_rrdlogs(LOGGING_READY), "Missing logging ready log"
    
    # Check configuration loading
    CONFIG_LOADED = "Configuration loaded"
    assert CONFIG_LOADED in grep_rrdlogs(CONFIG_LOADED), "Missing configuration loaded log"
    
    # Check MAC and timestamp
    MAC_TIMESTAMP = "MAC:"
    assert MAC_TIMESTAMP in grep_rrdlogs(MAC_TIMESTAMP), "Missing MAC address log"
    
    TIMESTAMP_LOG = "Timestamp:"
    assert TIMESTAMP_LOG in grep_rrdlogs(TIMESTAMP_LOG), "Missing timestamp log"
    
    # Check directory validation
    CHECKING_SIZE = "size and contents"
    assert CHECKING_SIZE in grep_rrdlogs(CHECKING_SIZE), "Missing directory size check log"
    
    LOG_DIR_VALIDATED = "Log directory validated and prepared"
    assert LOG_DIR_VALIDATED in grep_rrdlogs(LOG_DIR_VALIDATED), "Missing log directory validation log"
    
    # Check issue type sanitization
    ISSUE_SANITIZED = "Issue type sanitized"
    assert ISSUE_SANITIZED in grep_rrdlogs(ISSUE_SANITIZED), "Missing issue type sanitized log"
    
    # Check archive filename generation
    ARCHIVE_FILENAME = "Archive filename:"
    assert ARCHIVE_FILENAME in grep_rrdlogs(ARCHIVE_FILENAME), "Missing archive filename log"
    
    # Check archive creation
    CREATING_TARFILE = "Creating"
    TARFILE_TEXT = "tarfile from Debug Commands output"
    assert CREATING_TARFILE in grep_rrdlogs(CREATING_TARFILE), "Missing tarfile creation log"
    assert TARFILE_TEXT in grep_rrdlogs(TARFILE_TEXT), "Missing tarfile output log"
    
    # Check upload invocation
    INVOKING_UPLOAD = "Invoking uploadSTBLogs binary to upload"
    assert INVOKING_UPLOAD in grep_rrdlogs(INVOKING_UPLOAD), "Missing upload invocation log"
    
    UPLOAD_PARAMS = "uploadSTBLogs parameters - server:"
    assert UPLOAD_PARAMS in grep_rrdlogs(UPLOAD_PARAMS), "Missing upload parameters log"
    
    print("All rrd_upload_orchestrate logs validated successfully")

def test_remotedebugger_upload_report():
    print("Validating upload report logs...")
    
    # Check for upload result logs from rrd_upload_orchestrate
    UPLOAD_SUCCESS = "Debug Information Report upload Success"
    UPLOAD_FAILURE = "Debug Information Report upload Failed"
    
    if UPLOAD_SUCCESS in grep_rrdlogs(UPLOAD_SUCCESS):
        print("Upload successful - validating success path logs")
        
        # On success, check for cleanup logs
        REMOVING_REPORT = "Removing uploaded report"
        assert REMOVING_REPORT in grep_rrdlogs(REMOVING_REPORT), "Missing report removal log"
        
        # Check for orchestration exit
        ORCHESTRATE_EXIT = "Exit"
        assert ORCHESTRATE_EXIT in grep_rrdlogs(ORCHESTRATE_EXIT), "Missing orchestration exit log"
        
    elif UPLOAD_FAILURE in grep_rrdlogs(UPLOAD_FAILURE):
        print("Upload failed - validating failure path logs")
        # On failure, cleanup still happens but with different flow
        
    else:
        # Check legacy log messages for backward compatibility
        LEGACY_SUCCESS = "RRD Upload Script Execution Success"
        LEGACY_FAILURE = "RRD Upload Script Execution Failure"
        
        if LEGACY_SUCCESS in grep_rrdlogs(LEGACY_SUCCESS):
            print("Legacy upload success log found")
        elif LEGACY_FAILURE in grep_rrdlogs(LEGACY_FAILURE):
            print("Legacy upload failure log found")
        else:
            print("Warning: No upload status logs found")
    
    # Verify that the upload orchestration completed
    ORCHESTRATE_LOGS_PRESENT = (
        "Logging ready" in grep_rrdlogs("Logging ready") and
        "Configuration loaded" in grep_rrdlogs("Configuration loaded")
    )
    assert ORCHESTRATE_LOGS_PRESENT, "Upload orchestration did not execute properly"
    
    print("Upload report validation completed")
    
    # Cleanup
    remove_logfile()
    remove_outdir_contents(OUTPUT_DIR)
    kill_rrd()
