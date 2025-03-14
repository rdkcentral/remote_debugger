import json
from helper_functions import *

def test_check_remote_debugger_config_file():
    config_file_path = JSON_FILE
    assert check_file_exists(config_file_path), f"Configuration file '{config_file_path}' does not exist."

def test_check_rrd_directory_exists():
    dir_path = OUTPUT_DIR
    assert check_directory_exists(dir_path), f"Directory '{dir_path}' does not exist."

def test_check_dynamic_config_file():
    config_file_path = DYNAMIC_JSON_FILE
    assert check_file_exists(config_file_path), f"Configuration file '{config_file_path}' does not exist."

def test_check_dynamic_directory_exists():
    dir_path = DYNAMIC_DIR
    assert check_directory_exists(dir_path), f"Directory '{dir_path}' does not exist."

def check_output_dir():
    try:
        find_command = 'find /tmp/rrd/ -iname debug_outputs.txt'
        find_result = subprocess.run(find_command, shell=True, capture_output=True, text=True)
        if find_result.returncode == 0:
            files = find_result.stdout.strip().split('\n')
            for file in files:
                if file:  # Ensure the file path is not empty
                    cat_command = f'cat {file}'
                    cat_result = subprocess.run(cat_command, shell=True, capture_output=True, text=True)
                    if cat_result.returncode == 0 and cat_result.stdout.strip():
                        print(f"Contents of {file}:")
                        print(cat_result.stdout)
                        return "Success: File and content are present."
                    else:
                        print(f"Error reading {file} or file is empty: {cat_result.stderr}")
            return "Error: No valid debug_outputs.txt files found with content."
        else:
            return f"Error finding files: {find_result.stderr}"
    except Exception as e:
        return f"An error occurred: {e}"

def test_check_and_start_remotedebugger():
    kill_rrd()
    remove_logfile()
    print("Starting remotedebugger process")
    command_to_start = "nohup /usr/local/bin/remotedebugger > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    command_to_get_pid = "pidof remotedebugger"
    pid = run_shell_command(command_to_get_pid)
    assert pid != "", "remotedebugger process did not start"

def reset_issuetype_rfc():
    command = 'rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType string ""'
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    assert result.returncode == 0

def test_remote_debugger_trigger_event():
    reset_issuetype_rfc()
    sleep(10)
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', DYNAMIC_STRING
    ]
    result = subprocess.run(command, capture_output=True, text=True)
    assert result.returncode == 0

    sleep(15)

    QUERY_MSG = "Received event for RRD_SET_ISSUE_EVENT"
    assert QUERY_MSG in grep_rrdlogs(QUERY_MSG)

    MSG_SEND = "SUCCESS: Message sending Done"
    sleep(2)
    assert MSG_SEND in grep_rrdlogs(MSG_SEND)

    MSG_RECEIVE = "SUCCESS: Message Reception Done"
    sleep(2)
    assert MSG_RECEIVE in grep_rrdlogs(MSG_RECEIVE)

def test_check_issue_in_static_profile():
    READ_JSON = "Start Reading JSON File... /etc/rrd/remote_debugger.json"
    assert READ_JSON in grep_rrdlogs(READ_JSON)

    PARSE_JSON = "Static Profile Parse And Read Success"
    assert PARSE_JSON in grep_rrdlogs(PARSE_JSON)

    MISSING_MSG = "Issue Data Not found in Static JSON File"
    assert MISSING_MSG in grep_rrdlogs(MISSING_MSG)

def test_check_issue_in_dynamic_profile():
    DYNAMIC_READ = "Checking Dynamic Profile..."
    assert DYNAMIC_READ in grep_rrdlogs(DYNAMIC_READ)

    DYNAMIC_JSONFILE = "Reading json config file /media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json"
    assert DYNAMIC_JSONFILE in grep_rrdlogs(DYNAMIC_JSONFILE)

    PARSE_SUCCESS = "Dynamic Profile Parse And Read Success"
    assert PARSE_SUCCESS in grep_rrdlogs(PARSE_SUCCESS)

    SUBTYPE_MSG = "Reading all Sub Issue types"
    assert SUBTYPE_MSG in grep_rrdlogs(SUBTYPE_MSG)

    ISSUE_FOUND = f"Issue Data Node:{DYNAMIC_STRING}"
    assert ISSUE_FOUND in grep_rrdlogs(ISSUE_FOUND)

    DIR_CREATION = "Creating Directory"
    assert DIR_CREATION in grep_rrdlogs(DIR_CREATION)

    ALL_ISSUE_MSG = "Run Debug Commands for all issue types"
    assert ALL_ISSUE_MSG in grep_rrdlogs(ALL_ISSUE_MSG)

    TESTRUN1_MSG = "Reading Issue Type Test:TestRun1"
    assert TESTRUN1_MSG in grep_rrdlogs(TESTRUN1_MSG)

    TESTRUN1_CMD = "/proc/version"
    assert TESTRUN1_CMD in grep_rrdlogs(TESTRUN1_CMD)

    DEBUG_FILE = "Adding Details of Debug commands to Output File"
    assert DEBUG_FILE in grep_rrdlogs(DEBUG_FILE)

    testrun1_string = "Test.TestRun1"
    SERVICE_START = f"Starting remote_debugger_{testrun1_string} service success"
    assert SERVICE_START in grep_rrdlogs(SERVICE_START)

    JOURNAL_START = f"journalctl remote_debugger_{testrun1_string} service success"
    assert JOURNAL_START in grep_rrdlogs(JOURNAL_START)

    SLEEP_TIME = "Sleeping with timeout"
    assert SLEEP_TIME in grep_rrdlogs(SLEEP_TIME)
    sleep(20)

    SERVICE_STOP = f"Stopping remote_debugger_{testrun1_string} service"
    assert SERVICE_STOP in grep_rrdlogs(SERVICE_STOP)

    TESTRUN2_MSG = "Reading Issue Type Test:TestRun2"
    assert TESTRUN2_MSG in grep_rrdlogs(TESTRUN2_MSG)

    TESTRUN2_CMD = "cat /proc/buddyinfo;cat /proc/meminfo"
    assert TESTRUN2_CMD in grep_rrdlogs(TESTRUN2_CMD)

    DEBUG_FILE = "Adding Details of Debug commands to Output File"
    assert DEBUG_FILE in grep_rrdlogs(DEBUG_FILE)

    testrun2_string = "Test.TestRun2"
    SERVICE_START = f"Starting remote_debugger_{testrun2_string} service success"
    assert SERVICE_START in grep_rrdlogs(SERVICE_START)

    JOURNAL_START = f"journalctl remote_debugger_{testrun2_string} service success"
    assert JOURNAL_START in grep_rrdlogs(JOURNAL_START)

    SLEEP_TIME = "Sleeping with timeout"
    assert SLEEP_TIME in grep_rrdlogs(SLEEP_TIME)
    sleep(20)

    SERVICE_STOP = f"Stopping remote_debugger_{testrun2_string} service"
    assert SERVICE_STOP in grep_rrdlogs(SERVICE_STOP)

    result = check_output_dir()
    print(result)

    UPLOAD_LOGS = "Starting Upload Debug output Script: /lib/rdk/uploadRRDLogs.sh"
    assert UPLOAD_LOGS in grep_rrdlogs(UPLOAD_LOGS)

def test_remotedebugger_upload_report():
    UPLOAD_SUCCESS = "RRD Upload Script Execution Success"
    UPLOAD_FAILURE = "RRD Upload Script Execution Failure"
    if UPLOAD_SUCCESS in grep_rrdlogs(UPLOAD_SUCCESS):
        print("Upload success")
    elif UPLOAD_FAILURE in grep_rrdlogs(UPLOAD_FAILURE):
        print("Upload failed")
    else:
        print("Upload status not found in logs")

    SCRIPT_SUCCESS = "Debug Information Report upload Failed"
    SCRIPT_FAILURE = "Debug Information Report upload Success"
    if SCRIPT_SUCCESS in grep_rrdlogs(SCRIPT_SUCCESS):
        print("Script execution success")
    elif SCRIPT_FAILURE in grep_rrdlogs(SCRIPT_FAILURE):
        print("Script execution failed")
    else:
        print("Script execution not found in logs")

    remove_logfile()
    remove_outdir_contents(OUTPUT_DIR)
    kill_rrd()
