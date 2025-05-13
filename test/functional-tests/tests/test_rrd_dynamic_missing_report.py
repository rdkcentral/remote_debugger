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
    MISSING_STRING1 = "Test.TestRun5"
    reset_issuetype_rfc()
    sleep(10)
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', MISSING_STRING1
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

    PARSE_FAILED = "Dynamic Profile Parse/Read failed"
    assert PARSE_FAILED in grep_rrdlogs(PARSE_FAILED)

    RDM_MSG = "Request RDM Manager Download for a new Issue Type"
    assert RDM_MSG in grep_rrdlogs(RDM_MSG)

    RDM_PACKAGE = "Request RDM Manager Download for... RDK-RRD-Test:1.0"
    assert RDM_PACKAGE in grep_rrdlogs(RDM_PACKAGE)

    script_path="./test/functional-tests/tests/create_json.sh"
    # Run the shell script
    try:
        result = subprocess.run(['bash', script_path], check=True, text=True, capture_output=True)
        print("Script output:")
        print(result.stdout)
    except subprocess.CalledProcessError as e:
        print("Error while executing the script:")
        print(e.stderr)
def test_rdm_trigger_event():
    INSTALL_PACKAGE ="RDK-RRD-Test:1.0"
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RDKDownloadManager.InstallPackage',
        'string', INSTALL_PACKAGE
    ]
    result = subprocess.run(command, capture_output=True, text=True)
    assert result.returncode == 0
def test_rdm_rrd_trigger_event():
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.DownloadStatus',
        'bool', 'true'
    ]
    result = subprocess.run(command, capture_output=True, text=True)
    assert result.returncode == 0

def test_rdm_rrd_dwnld_status():
    PACKAGE_FOUND = "Package found in Cache...Test.TestRun5"
    assert PACKAGE_FOUND in grep_rrdlogs(PACKAGE_FOUND)

    PACKAGE_DETAIL = "Package Details jsonPath: /tmp/RDK-RRD-Test"
    assert PACKAGE_DETAIL in grep_rrdlogs(PACKAGE_DETAIL)

    COPY_MSG = "Copying Message Received to the queue.."
    assert COPY_MSG in grep_rrdlogs(COPY_MSG)

    SUCCESS_MSG_SEND = "Message sending Done, ID=0 MSG=Test.TestRun5 Size=13 Type=1 AppendMode=0!"
    assert SUCCESS_MSG_SEND in grep_rrdlogs(SUCCESS_MSG_SEND)

    SUCCESS_MSG_RECEIVE = "Message Reception Done for ID=0 MSG=Test.TestRun5 TYPE=1..."
    assert SUCCESS_MSG_RECEIVE in grep_rrdlogs(SUCCESS_MSG_RECEIVE)

    ISSUE_CMD_INFO = "Getting Issue command Information for : Test.TestRun5"
    assert ISSUE_CMD_INFO in grep_rrdlogs(ISSUE_CMD_INFO)

    CHECK_IN_DYNAMIC = "Checking if Issue marked inDynamic..."
    assert CHECK_IN_DYNAMIC in grep_rrdlogs(CHECK_IN_DYNAMIC)

    ISSUE_MARKED_DYNAMIC = "Issue Marked as inDynamic..."
    assert ISSUE_MARKED_DYNAMIC in grep_rrdlogs(ISSUE_MARKED_DYNAMIC)

    START_JSON_READ = "Start Reading JSON File... /tmp/RDK-RRD-Test/etc/rrd/remote_debugger.json"
    assert START_JSON_READ in grep_rrdlogs(START_JSON_READ)

    JSON_READ_SUCCESS = "Reading json file Success, Parsing the Content..."
    assert JSON_READ_SUCCESS in grep_rrdlogs(JSON_READ_SUCCESS)

    JSON_PARSE_SUCCESS = "Json File parse Success... /tmp/RDK-RRD-Test/etc/rrd/remote_debugger.json"
    assert JSON_PARSE_SUCCESS in grep_rrdlogs(JSON_PARSE_SUCCESS)

    READ_ISSUE_CATEGORY = "Reading Issue Category:Test..."
    assert READ_ISSUE_CATEGORY in grep_rrdlogs(READ_ISSUE_CATEGORY)

    RUN_DEBUG_CMD = "Run Debug Commands for Test:TestRun5"
    assert RUN_DEBUG_CMD in grep_rrdlogs(RUN_DEBUG_CMD)

    EXEC_RUNTIME_SERVICE = "Executing Commands in Runtime Service..."
    assert EXEC_RUNTIME_SERVICE in grep_rrdlogs(EXEC_RUNTIME_SERVICE)

    START_REMOTE_DEBUGGER = "Starting remote_debugger_Test.TestRun5 service success..."
    assert START_REMOTE_DEBUGGER in grep_rrdlogs(START_REMOTE_DEBUGGER)

    remove_logfile()
    remove_outdir_contents(OUTPUT_DIR)
    kill_rrd()
