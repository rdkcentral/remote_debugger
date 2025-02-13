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

