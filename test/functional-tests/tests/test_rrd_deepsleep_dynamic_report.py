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
    command_to_start = "nohup ./test/functional-tests/tests/test_pwr_event_handler > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    command_to_get_pid = "pidof test_pwr_event_handler"
    pid = run_shell_command(command_to_get_pid)
    assert pid != "", "remotedebugger process did not start"

def test_deepsleep_trigger():
    sleep(10)
    CURR_STATE_1 = "Entering.. currentState =5, newState = 5"
    assert CURR_STATE_1 in grep_rrdlogs(CURR_STATE_1)

    DEEP_SLEEP_EVENT = "Event Triggred for DeepSleep"
    assert DEEP_SLEEP_EVENT in grep_rrdlogs(DEEP_SLEEP_EVENT)

    CURR_STATE_2 = "...Entering.. currentState =5, newState = 3"
    assert CURR_STATE_2 in grep_rrdlogs(CURR_STATE_2)

    RECEIVED_STATE = "Received state from Power Manager Current :[5] New[3]"
    assert RECEIVED_STATE in grep_rrdlogs(RECEIVED_STATE)

    COPY_QUEUE = "Copying Message Received to the queue"
    assert COPY_QUEUE in grep_rrdlogs(COPY_QUEUE)


def test_message_reception():
    MSG_RECEIVE_DEEPSLEEP = "Message Reception Done for ID=0 MSG=DEEPSLEEP TYPE=3"
    assert MSG_RECEIVE_DEEPSLEEP in grep_rrdlogs(MSG_RECEIVE_DEEPSLEEP)

    GET_ISSUE_INFO = "Getting Issue command Information for : DEEPSLEEP"
    assert GET_ISSUE_INFO in grep_rrdlogs(GET_ISSUE_INFO)

    SEND_RDM_REQ = "Sending RDM Download Request for DeepSleep dynamic package..."
    assert SEND_RDM_REQ in grep_rrdlogs(SEND_RDM_REQ)

    REQ_RDM_MANAGER = "Request RDM Manager Download for a new Issue Type"
    assert REQ_RDM_MANAGER in grep_rrdlogs(REQ_RDM_MANAGER)

    APPEND_ITEM = "Append Item with PkgData: RDK-RRD-DEEPSLEEP and issue Type: DEEPSLEEP to Cache"
    assert APPEND_ITEM in grep_rrdlogs(APPEND_ITEM)

    SET_PARAMS_SUCCESS = "Setting Parameters Success and Cache Updated ...RDK-RRD-DEEPSLEEP IssueStr:DEEPSLEEP Length:9"
    assert SET_PARAMS_SUCCESS in grep_rrdlogs(SET_PARAMS_SUCCESS)


def trigger_rdm_for_deepsleep():

    EVENT_RECEIVED = "Received event for IARM_BUS_RDMMGR_NAME RDMMgr"
    assert EVENT_RECEIVED in grep_rrdlogs(EVENT_RECEIVED)

    FIND_PKG_CACHE = "finding RDK-RRD-DEEPSLEEP PkgData in Cache"
    assert FIND_PKG_CACHE in grep_rrdlogs(FIND_PKG_CACHE)

    PKG_FOUND = "Package found in Cache...DEEPSLEEP"
    assert PKG_FOUND in grep_rrdlogs(PKG_FOUND)

    INSTALL_COMPLETE = "Package Installation Status Complete"
    assert INSTALL_COMPLETE in grep_rrdlogs(INSTALL_COMPLETE)

    MSG_SEND_DEEPSLEEP = "Message sending Done, ID=0 MSG=DEEPSLEEP Size=9 Type=3 AppendMode=0!"
    assert MSG_SEND_DEEPSLEEP in grep_rrdlogs(MSG_SEND_DEEPSLEEP)

def test_command_execute():
    MAIN_NODE = "Received Main Node= DEEPSLEEP"
    assert MAIN_NODE in grep_rrdlogs(MAIN_NODE)

    READ_JSON_SUCCESS = "Reading json file Success, Parsing the Content..."
    assert READ_JSON_SUCCESS in grep_rrdlogs(READ_JSON_SUCCESS)

    RUN_DEBUG_CMDS = "Run Debug Commands for all issue types in  DEEPSLEEP"
    assert RUN_DEBUG_CMDS in grep_rrdlogs(RUN_DEBUG_CMDS)

    READ_ISSUE_TYPE = "Reading Issue Type 1:Audio.Audio"
    assert READ_ISSUE_TYPE in grep_rrdlogs(READ_ISSUE_TYPE)

    FOUND_VALID_CMDS = "Found valid Commands, Execute..."
    assert FOUND_VALID_CMDS in grep_rrdlogs(FOUND_VALID_CMDS)

    EXECUTE_CMDS = "Executing Commands in Runtime Service.."
    assert EXECUTE_CMDS in grep_rrdlogs(EXECUTE_CMDS)

    START_SERVICE_SUCCESS = "Starting remote_debugger_Audio.Audio service success..."
    assert START_SERVICE_SUCCESS in grep_rrdlogs(START_SERVICE_SUCCESS)

    JOURNALCTL_SUCCESS = "journalctl remote_debugger_Audio.Audio service success..."
    assert JOURNALCTL_SUCCESS in grep_rrdlogs(JOURNALCTL_SUCCESS)

    SLEEP_BEFORE_STOP = "Sleeping with timeout 10 sec before stopping service..."
    assert SLEEP_BEFORE_STOP in grep_rrdlogs(SLEEP_BEFORE_STOP)


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
