import json
from helper_functions import *

def test_check_remote_debugger_config_file():
    config_file_path = JSON_FILE
    assert check_file_exists(config_file_path), f"Configuration file '{config_file_path}' does not exist."

def test_check_rrd_directory_exists():
    dir_path = OUTPUT_DIR
    assert check_directory_exists(dir_path), f"Directory '{dir_path}' does not exist."

def test_check_dynamic_config_file():
    config_file_path = APPEND_JSON_FILE
    assert check_file_exists(config_file_path), f"Configuration file '{config_file_path}' does not exist."

def test_check_dynamic_directory_exists():
    dir_path = DYNAMIC_DIR
    assert check_directory_exists(dir_path), f"Directory '{dir_path}' does not exist."

def test_check_and_start_remotedebugger():
    kill_rrd()
    remove_logfile()
    test_check_dynamic_directory_exists()
    test_check_dynamic_config_file()
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
        'string', APPEND_STRING
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

def test_check_issue_in_dynamic_profile():
    APPEND_MSG = "Received append request to process static and dynamic profiles"
    assert APPEND_MSG in grep_rrdlogs(APPEND_MSG)

    DYNAMIC_JSONFILE = "Reading json config file /media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json"
    assert DYNAMIC_JSONFILE in grep_rrdlogs(DYNAMIC_JSONFILE)

    JSON_READ_SUCCESS = "Reading json file Success"
    assert JSON_READ_SUCCESS in grep_rrdlogs(JSON_READ_SUCCESS)

    JSON_PARSE_SUCCESS = "Json File parse Success"
    assert JSON_PARSE_SUCCESS in grep_rrdlogs(JSON_PARSE_SUCCESS)

    JSON_SUCCESS = "Dynamic Profile Parse And Read Success... /media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json"
    assert JSON_SUCCESS in grep_rrdlogs(JSON_SUCCESS)

    CHECKING_DYNAMIC_JSON = "Check if Issue in Parsed Dynamic JSON... /media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json"
    assert CHECKING_DYNAMIC_JSON in grep_rrdlogs(CHECKING_DYNAMIC_JSON)

    STATIC_READ = "Reading static profile command info..."
    assert STATIC_READ in grep_rrdlogs(STATIC_READ)

    READING_JSON = "Start Reading JSON File... /etc/rrd/remote_debugger.json"
    assert READING_JSON in grep_rrdlogs(READING_JSON)

    JSON_STATIC_SUCCESS = "Reading json file Success, Parsing the Content..."
    assert JSON_STATIC_SUCCESS in grep_rrdlogs(JSON_STATIC_SUCCESS)

    JSON_PARSE_STATIC_SUCCESS = "Json File parse Success... /etc/rrd/remote_debugger.json"
    assert JSON_PARSE_STATIC_SUCCESS in grep_rrdlogs(JSON_PARSE_STATIC_SUCCESS)

    JSON_SUCCESS_STATIC = "Static Profile Parse And Read Success... /etc/rrd/remote_debugger.json"
    assert JSON_SUCCESS_STATIC in  grep_rrdlogs(JSON_SUCCESS_STATIC)

    CHECKING_STATIC_JSON = "Check if Issue in Parsed Static JSON... /etc/rrd/remote_debugger.json"
    assert CHECKING_STATIC_JSON in grep_rrdlogs(CHECKING_STATIC_JSON)

    READ_COMPLETE_STATIC = "Issue Category:Test not found in JSON File!!!"
    assert READ_COMPLETE_STATIC in grep_rrdlogs(READ_COMPLETE_STATIC)

    STATIC_NOTFOUND = "Static Command Info not found for IssueType!!!"
    assert STATIC_NOTFOUND in grep_rrdlogs(STATIC_NOTFOUND)


    remove_logfile()
    remove_outdir_contents(OUTPUT_DIR)
    kill_rrd()
