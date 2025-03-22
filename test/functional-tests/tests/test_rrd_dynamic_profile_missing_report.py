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
    reset_issuetype_rfc()
    sleep(10)
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', MISSING_STRING
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

    script_path="create_json.sh"
# Run the shell script
    try:
        result = subprocess.run(['bash', script_path], check=True, text=True, capture_output=True)
        print("Script output:")
        print(result.stdout)
    except subprocess.CalledProcessError as e:
        print("Error while executing the script:")
        print(e.stderr)
    script_path = "/root/rrd/remote_debugger/test/functional-tests/tests/create_json.sh"
    try:
        result = subprocess.run(['bash', script_path], check=True, text=True, capture_output=True)
        print("Script output:")
        print(result.stdout)
    except subprocess.CalledProcessError as e:
        print("Error while executing the script:")
        print(f"Return code: {e.returncode}")
        print(f"Command: {e.cmd}")
        print(f"Output: {e.output}")
        print(f"Error: {e.stderr}")
    except FileNotFoundError as fnf_error:
        print("FileNotFoundError: The script path might be incorrect.")
        print(fnf_error)



#    remove_logfile()
    remove_outdir_contents(OUTPUT_DIR)
    kill_rrd()

#def download_file(filename):
#    url = f"https://mockxconf:50054/tmp/{filename}"
#    command = f"curl -k -o /tmp/rrd/{filename} {url}"
#    print(f"Executing command: {command}")  # Debugging line

#    result = subprocess.run(command, shell=True, capture_output=True, text=True)
#    return result

#tgz_file = "RDK-RRD-Test.tar"
#download_file(tgz_file)


#def download_file(filename):
#    url = f"https://mockxconf:50054/tmp/{filename}"
#    command = f"curl -k -o /media/apps/RDK-RRD-Test/etc/rrd/{filename} {url}"
#    print(f"Executing command: {command}")  # Debugging line

#    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    #command = f"tar -xvf /media/apps/RDK-RRD-Test/etc/rrd/{filename}"
    #result = subprocess.run(command, shell=True, capture_output=True, text=True)
    #command = f"mv /media/apps/RDK-RRD-Test/etc/rrd/{filename} /media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json"
    #result = subprocess.run(command, shell=True, capture_output=True, text=True)
#    return result

#tgz_file = "RDK-RRD-Test.tar.gz"
#download_file(tgz_file)
def test_rdm_trigger_event():
    INSTALL_PACKAGE ="RDK-RRD-Test:1.0"
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RDKDownloadManager.InstallPackage',
        'string', INSTALL_PACKAGE
    ]
    result = subprocess.run(command, capture_output=True, text=True)
    assert result.returncode == 0
