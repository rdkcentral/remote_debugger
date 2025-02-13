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
        'string', BACKGROUND_STRING
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

    ISSUE_MSG = f'MSG={BACKGROUND_STRING}'
    assert ISSUE_MSG in grep_rrdlogs(ISSUE_MSG)
    print("Sent and received messages are found and match in the logfile")

    READ_JSON = "Start Reading JSON File... /etc/rrd/remote_debugger.json"
    assert READ_JSON in grep_rrdlogs(READ_JSON)

    PARSE_JSON = "Json File parse Success... /etc/rrd/remote_debugger.json"
    assert PARSE_JSON in grep_rrdlogs(PARSE_JSON)

    if '.' in BACKGROUND_STRING:
        ISSUE_NODE, ISSUE_SUBNODE = BACKGROUND_STRING.split('.')
    else:
        node = BACKGROUND_STRING
        subnode = None

    ISSUE_FOUND = f"Issue Data Node: {ISSUE_NODE} and Sub-Node: {ISSUE_SUBNODE} found in Static JSON File /etc/rrd/remote_debugger.json"

    DIR_CREATION = "Creating Directory"
    assert DIR_CREATION in grep_rrdlogs(DIR_CREATION)

    SANITY_CHECK = "Found valid Commands"
    assert SANITY_CHECK in grep_rrdlogs(SANITY_CHECK)

    BG_CHECK = "Received Commands to execute in background"
    assert BG_CHECK in grep_rrdlogs(BG_CHECK)

    REPLACE_MSG = "Replacing default location"
    assert REPLACE_MSG in grep_rrdlogs(REPLACE_MSG)

    COMMAND_CHECK = "tcpdump -w RRD_LOCATION/capture.pcap"
    assert COMMAND_CHECK in grep_rrdlogs(COMMAND_CHECK)

    REPLACE_MSG = "Replacing default location"
    assert REPLACE_MSG in grep_rrdlogs(REPLACE_MSG)

    REPLACE_MSG = "Updated Command from replaceRRDLocation"
    assert REPLACE_MSG in grep_rrdlogs(REPLACE_MSG)

    DEBUG_FILE = "Adding Details of Debug commands to Output File"
    assert DEBUG_FILE in grep_rrdlogs(DEBUG_FILE)

    SERVICE_START = f"Starting remote_debugger_{BACKGROUND_STRING} service success"
    assert SERVICE_START in grep_rrdlogs(SERVICE_START)

    JOURNAL_START = f"journalctl remote_debugger_{BACKGROUND_STRING} service success"
    assert JOURNAL_START in grep_rrdlogs(JOURNAL_START)

    SLEEP_TIME = "Sleeping with timeout"
    assert SLEEP_TIME in grep_rrdlogs(SLEEP_TIME)
    sleep(20)

    SERVICE_STOP = f"Stopping remote_debugger_{BACKGROUND_STRING} service"
    assert SERVICE_STOP in grep_rrdlogs(SERVICE_STOP)

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

    remove_logfile()
    remove_outdir_contents(OUTPUT_DIR)
    kill_rrd()
