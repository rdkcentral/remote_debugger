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

import subprocess
import requests
import os
import time
import re
import signal
import shutil
from time import sleep

RRD_LOG_FILE = "/opt/logs/remotedebugger.log.0"
LOG_FILE = "/opt/logs/remotedebugger.log.0"
JSON_FILE = "/etc/rrd/remote_debugger.json"
UPLOAD_SCRIPT = "/lib/rdk/uploadRRDLogs.sh"
OUTPUT_DIR = "/tmp/rrd"
ISSUE_STRING = "Device.Info"
CATEGORY_STRING = "Device"
HARMFULL_STRING = "Command.Harm"
MISSING_STRING = "Test.TestRun"
BACKGROUND_STRING = "Device.Dump"
DYNAMIC_DIR="/media/apps/RDK-RRD-Test/etc/rrd"
DYNAMIC_JSON_FILE = "/media/apps/RDK-RRD-Device/etc/rrd/remote_debugger.json"
DYNAMIC_STRING = "Test"
DYNAMIC_HARMFUL_STRING = "Test.TestRun3"
APPEND_STRING ="Test.TestRun2_apnd"
APPEND_JSON_FILE ="/media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json"
def remove_logfile():
    try:
        if os.path.exists(RRD_LOG_FILE):
            os.remove(RRD_LOG_FILE)
            print(f"Log file {RRD_LOG_FILE} removed.")
        else:
            print(f"Log file {RRD_LOG_FILE} does not exist.")
    except Exception as e:
        print(f"Could not remove log file {RRD_LOG_FILE}: {e}")


def kill_rrd(signal: int=9):
    print(f"Received Signal to kill remotedebugger {signal} with pid {get_pid('remotedebugger')}")
    resp = subprocess.run(f"kill -{signal} {get_pid('remotedebugger')}", shell=True, capture_output=True)
    print(resp.stdout.decode('utf-8'))
    print(resp.stderr.decode('utf-8'))
    return ""

def grep_rrdlogs(search: str):
    search_result = ""
    search_pattern = re.compile(re.escape(search), re.IGNORECASE)
    try:
        with open(LOG_FILE, 'r', encoding='utf-8', errors='ignore') as file:
            for line_number, line in enumerate(file, start=1):
                if search_pattern.search(line):
                    search_result = search_result + " \n" + line
    except Exception as e:
        print(f"Could not read file {LOG_FILE}: {e}")
    return search_result

def check_file_exists(file_path):
    return os.path.isfile(file_path)

def check_directory_exists(dir_path):
    return os.path.isdir(dir_path)

def is_remotedebugger_running():
    command_to_check = "ps aux | grep remotedebugger | grep -v grep"
    result = run_shell_command(command_to_check)
    return result != ""

def get_pid(name: str):
    return subprocess.run(f"pidof {name}", shell=True, capture_output=True).stdout.decode('utf-8').strip()

def run_shell_silent(command):
    subprocess.run(command, shell=True, capture_output=False, text=False)
    return

def run_shell_command(command):
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    return result.stdout.strip()

def get_issue_type():
    command = [
        'rbuscli', 'get',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType'
    ]
    result = subprocess.run(command, capture_output=True, text=True)
    assert result.returncode == 0
    return result.stdout.strip()

def remove_outdir_contents(directory):
    if os.path.exists(directory):
        for filename in os.listdir(directory):
            file_path = os.path.join(directory, filename)
            try:
                if os.path.isfile(file_path) or os.path.islink(file_path):
                    os.unlink(file_path)
                elif os.path.isdir(file_path):
                    shutil.rmtree(file_path)
            except Exception as e:
                print(f'Failed to delete {file_path}. Reason: {e}')

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
