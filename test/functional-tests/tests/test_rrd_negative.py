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
    MISSING_STRING_NEG = "Test.TestRun7"
    reset_issuetype_rfc()
    sleep(10)
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', MISSING_STRING_NEG
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

def test_rdm_trigger_event():
    INSTALL_PACKAGE ="RDK-RRD-Test:1.0"
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RDKDownloadManager.InstallPackage',
        'string', INSTALL_PACKAGE
    ]
    result = subprocess.run(command, capture_output=True, text=True)
    assert result.returncode == 0

    RBUS_SETTING_SUCCESS = "Setting Parameters using rbus success..."
    assert RBUS_SETTING_SUCCESS in grep_rrdlogs(RBUS_SETTING_SUCCESS)

    CACHE_UPDATE_SUCCESS = "Setting Parameters Success and Cache Updated ...RDK-RRD-Test"
    assert CACHE_UPDATE_SUCCESS in grep_rrdlogs(CACHE_UPDATE_SUCCESS)

def test_rdm_download_event():
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RDKDownloadManager.DownloadStatus',
        'bool', 'true'
    ]
    result = subprocess.run(command, capture_output=True, text=True)
    assert result.returncode == 0
    sleep(2)

    RDM_DOWNLOAD_EVENT = "Received event for RDM_DOWNLOAD_EVENT Device.DeviceInfo.X_RDKCENTRAL-COM_RDKDownloadManager.DownloadStatus"
    assert RDM_DOWNLOAD_EVENT in grep_rrdlogs(RDM_DOWNLOAD_EVENT)

    PACKAGE_FOUND = "Package found in Cache...Test.TestRun7"
    assert PACKAGE_FOUND in grep_rrdlogs(PACKAGE_FOUND)

    MESSAGE_SUCCESS = "SUCCESS: Message sending Done, ID=0 MSG=Test.TestRun7 Size=13 Type=1 AppendMode=0!"
    assert MESSAGE_SUCCESS in grep_rrdlogs(MESSAGE_SUCCESS)

    ISSUE_TYPE_LIST = "IssueType List [Test.TestRun7]..."
    assert ISSUE_TYPE_LIST in grep_rrdlogs(ISSUE_TYPE_LIST)

    CHECKING_DYNAMIC = "Checking if Issue marked inDynamic..."
    assert CHECKING_DYNAMIC in grep_rrdlogs(CHECKING_DYNAMIC)

    READING_JSON_CONFIG = "Reading json config file /tmp/RDK-RRD-Test/etc/rrd/remote_debugger.json"
    assert READING_JSON_CONFIG in grep_rrdlogs(READING_JSON_CONFIG)

    JSON_PARSE_SUCCESS = "Reading json file Success, Parsing the Content..."
    assert JSON_PARSE_SUCCESS in grep_rrdlogs(JSON_PARSE_SUCCESS)

    DYNAMIC_JSON_PARSE_FAILED = "Dynamic JSON Parse/Read failed... /tmp/RDK-RRD-Test/etc/rrd/remote_debugger.json"
    assert DYNAMIC_JSON_PARSE_FAILED in grep_rrdlogs(DYNAMIC_JSON_PARSE_FAILED)

    remove_logfile()
    remove_outdir_contents(OUTPUT_DIR)
    kill_rrd()
