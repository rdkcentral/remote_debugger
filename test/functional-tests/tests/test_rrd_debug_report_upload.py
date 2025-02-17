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

def get_rrd_tarfile():
    logfile = '/opt/logs/remotedebugger.log.0'
    command = f"grep 'uploadSTBLogs.sh' {logfile} | grep -oP '\\S+\\.tgz'"
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    if result.returncode == 0:
        return result.stdout.strip()
    return None

def download_file(filename):
    url = f"https://mockxconf:50054/tmp/{filename}"
    command = f"curl -k -o {filename} {url}"
    print(f"Executing command: {command}")  # Debugging line

    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    return result

def validate_file(filename):
    if os.path.isfile(filename):
        os.remove(filename)
        return f"File {filename} found and removed."
    else:
        return f"File {filename} not found."

def check_server_status():
    url = "https://mockxconf:50054/rrdDebugReport"
    result = subprocess.run(["curl", "-k", "-I", "-w", "%{http_code}", url, "-o", "/dev/null"], capture_output=True, text=True)
    status_code = result.stdout.strip()
    return status_code

def test_check_and_start_remotedebugger():
    kill_rrd()
    status_code = check_server_status()
    assert status_code == "200", f"Unexpected status code: {status_code}"

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

    UPLOAD_LOGS = "Starting Upload Debug output Script: /lib/rdk/uploadRRDLogs.sh"
    assert UPLOAD_LOGS in grep_rrdlogs(UPLOAD_LOGS)


def test_remotedebugger_upload_report():
    tgz_file = get_rrd_tarfile()
    assert tgz_file is not None, "No .tgz files found in the directory"
    print(f"Found .tgz file: {tgz_file}")

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

def test_remotedebugger_download_report():
    tgz_file = get_rrd_tarfile()
    result = download_file(tgz_file)
    assert result.returncode == 0, f"Download failed: {result.stderr}"
    print(f"Download successful: {result.stdout}")

    message = validate_file(tgz_file)
    print(message)

    kill_rrd()
    remove_logfile()
    remove_outdir_contents(OUTPUT_DIR)
