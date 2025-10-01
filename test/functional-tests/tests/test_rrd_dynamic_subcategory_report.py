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

import json
import subprocess
from helper_functions import *

def test_check_remote_debugger_config_file():
    config_file_path = JSON_FILE
    assert check_file_exists(config_file_path), f"Configuration file '{config_file_path}' does not exist."

def test_check_rrd_directory_exists():
    dir_path = OUTPUT_DIR
    assert check_directory_exists(dir_path), f"Directory '{dir_path}' does not exist."

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
    STRING_TEST = "Test.TestRun1"
    reset_issuetype_rfc()
    sleep(10)
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', STRING_TEST
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

    script_path="./test/functional-tests/tests/create_json.sh"
    # Run the shell script
    try:
        result = subprocess.run(['bash', script_path], check=True, text=True, capture_output=True)
        print("Script output:")
        print(result.stdout)
    except subprocess.CalledProcessError as e:
        print("Error while executing the script:")
        print(e.stderr)

def test_check_issue_in_dynamic_profile():
    DYNAMIC_READ = "Checking Dynamic Profile..."
    assert DYNAMIC_READ in grep_rrdlogs(DYNAMIC_READ)

    DYNAMIC_JSONFILE = "Reading json config file /media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json"
    assert DYNAMIC_JSONFILE in grep_rrdlogs(DYNAMIC_JSONFILE)

    DYNAMIC_READ = "Reading json file Success, Parsing the Content..."
    assert DYNAMIC_READ in grep_rrdlogs(DYNAMIC_READ)

    DYNAMIC_PROFILE = "Dynamic Profile Parse And Read Success... /media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json"
    assert DYNAMIC_PROFILE in grep_rrdlogs(DYNAMIC_PROFILE)

    CHECK_PARSED_JSON = "Check if Issue in Parsed Dynamic JSON... /media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json"
    assert CHECK_PARSED_JSON in grep_rrdlogs(CHECK_PARSED_JSON)

    READING_CATEGORY = "Reading Issue Category:Test..."
    assert READING_CATEGORY in grep_rrdlogs(READING_CATEGORY)

    ISSUE_DATA_NODE = "Issue Data Node:Test and Sub-Node:TestRun1 found in Dynamic JSON File /media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json..."
    assert ISSUE_DATA_NODE in grep_rrdlogs(ISSUE_DATA_NODE)

    CREATE_DIR = "Creating Directory /tmp/rrd/Test-DebugReport"
    assert CREATE_DIR in grep_rrdlogs(CREATE_DIR)

    RUN_DEBUG = "Run Debug Commands for Test:TestRun1"
    assert RUN_DEBUG in grep_rrdlogs(RUN_DEBUG)

    READ_SANITY = "Reading Sanity Check Commands List"
    assert READ_SANITY in grep_rrdlogs(READ_SANITY)

    FOUND_COMMANDS = "Found valid Commands, Execute..."
    assert FOUND_COMMANDS in grep_rrdlogs(FOUND_COMMANDS)

    EXEC_RUNTIME = "Executing Commands in Runtime Service..."
    assert EXEC_RUNTIME in grep_rrdlogs(EXEC_RUNTIME)

    EXEC_COMMANDS = 'Executing Debug Commands: ""cat /version.txt;uptime;cat /proc/buddyinfo;cat /proc/meminfo;cat /tmp/.deviceDetails.cache""'
    assert EXEC_COMMANDS in grep_rrdlogs(EXEC_COMMANDS)

    START_SERVICE = "Starting remote_debugger_Test.TestRun1 service success..."
    assert START_SERVICE in grep_rrdlogs(START_SERVICE)

    USE_JOURNALCTL = "Using journalctl to log command output..."
    assert USE_JOURNALCTL in grep_rrdlogs(USE_JOURNALCTL)

    JOURNALCTL_SUCCESS = "journalctl remote_debugger_Test.TestRun1 service success..."
    assert JOURNALCTL_SUCCESS in grep_rrdlogs(JOURNALCTL_SUCCESS)

    STOP_SERVICE = "Stopping remote_debugger_Test.TestRun1 service..."
    assert STOP_SERVICE in grep_rrdlogs(STOP_SERVICE)

    UPLOAD_SCRIPT_START = "Starting Upload Debug output Script: /lib/rdk/uploadRRDLogs.sh..."
    assert UPLOAD_SCRIPT_START in grep_rrdlogs(UPLOAD_SCRIPT_START)

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
