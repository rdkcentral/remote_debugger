import json
from helper_functions import *

# Path to the existing JSON file
file_path = "/etc/rrd/remote_debugger.json"

# Read the existing JSON data
with open(file_path, "r") as json_file:
    data = json.load(json_file)

# New entry to add
new_entry = {
    "Test": {
        "TestRun4": {
            "Commands": "cat /version.txt;cat /tmp/.deviceDetails.cache",
            "Timeout": 10
        }
    }
}

# Update the JSON data with the new entry
data.update(new_entry)

# Write the updated data back to the JSON file
with open(file_path, "w") as json_file:
    json.dump(data, json_file, indent=4)

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
    APPEND_STRING1 = "Test.TestRun4_apnd"
    reset_issuetype_rfc()
    sleep(10)
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', APPEND_STRING1
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

    SUCCESS_STATIC = "Static Profile Parse And Read Success... /etc/rrd/remote_debugger.json"
    assert SUCCESS_STATIC in  grep_rrdlogs(SUCCESS_STATIC)

    CHECKING_STATIC_JSON = "Check if Issue in Parsed Static JSON... /etc/rrd/remote_debugger.json"
    assert CHECKING_STATIC_JSON in grep_rrdlogs(CHECKING_STATIC_JSON)

    READ_COMPLETE_STATIC = "Read complete for Static Profile: RFCValue: Test.TestRun4, Command: "
    assert READ_COMPLETE_STATIC in grep_rrdlogs(READ_COMPLETE_STATIC)

    APPEND_UPDATE = "Updated command after append from dynamic and static profile: "
    assert APPEND_UPDATE in grep_rrdlogs(APPEND_UPDATE)

    EXECUTE_SERVICE = "Executing Commands in Runtime Service..."
    assert EXECUTE_SERVICE in grep_rrdlogs(EXECUTE_SERVICE)

    remove_logfile()
    remove_outdir_contents(OUTPUT_DIR)
    kill_rrd()

