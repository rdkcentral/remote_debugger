import json
from helper_functions import *

def test_check_remote_debugger_config_file():
    config_file_path = JSON_FILE
    assert check_file_exists(config_file_path), f"Configuration file '{config_file_path}' does not exist."

def test_check_rrd_directory_exists():
    dir_path = OUTPUT_DIR
    assert check_directory_exists(dir_path), f"Directory '{dir_path}' does not exist."

def test_check_and_start_remotedebugger():
    kill_rrd()
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
        'string', HARMFULL_STRING
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

    READ_JSON = "Start Reading JSON File... /etc/rrd/remote_debugger.json"
    assert READ_JSON in grep_rrdlogs(READ_JSON)

    PARSE_JSON = "Static Profile Parse And Read Success"
    assert PARSE_JSON in grep_rrdlogs(PARSE_JSON)

    CATEGORY_MSG = "Reading Issue Category"
    assert CATEGORY_MSG in grep_rrdlogs(CATEGORY_MSG)

    TYPE_MSG = "Reading Issue Type"
    assert TYPE_MSG in grep_rrdlogs(TYPE_MSG)

    if '.' in HARMFULL_STRING:
        ISSUE_NODE, ISSUE_SUBNODE = HARMFULL_STRING.split('.')
    else:
        node = HARMFULL_STRING
        subnode = None

    ISSUE_FOUND = f"Issue Data Node: {ISSUE_NODE} and Sub-Node: {ISSUE_SUBNODE} found in Static JSON File /etc/rrd/remote_debugger.json"

    DIR_CREATION = "Creating Directory"
    assert DIR_CREATION in grep_rrdlogs(DIR_CREATION)

    READ_COMMAND_MSG = "Reading Command and Timeout information for Debug Issue"
    assert READ_COMMAND_MSG in grep_rrdlogs(READ_COMMAND_MSG)

def test_remote_debugger_command_sanity():
    VALID_COMMAND = "rm -rf;kill;pkill;iptables;ip6tables"
    assert VALID_COMMAND in grep_rrdlogs(VALID_COMMAND)

    SANITY_CHECK = "Found harmful commands"
    assert SANITY_CHECK in grep_rrdlogs(SANITY_CHECK)

    sleep(5)
    ABORT_MSG = "Aborting Command execution due to Harmful commands"
    assert ABORT_MSG in grep_rrdlogs(ABORT_MSG)

    UPLOAD_MSG = "Skip uploading Debug Report"
    assert UPLOAD_MSG in grep_rrdlogs(UPLOAD_MSG)

    remove_logfile()
    remove_outdir_contents(OUTPUT_DIR)
    kill_rrd()
