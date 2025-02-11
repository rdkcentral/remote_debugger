from time import sleep
from helper_functions import *

def test_check_remote_debugger_config_file():
    config_file_path = JSON_FILE
    assert check_file_exists(config_file_path)

def test_check_rrd_directory_exists():
    dir_path = OUTPUT_DIR
    assert check_directory_exists(dir_path)

def test_check_remotedebugger_is_starting():
    kill_rrd()
    remove_logfile()
    print("Starting remotedebugger process")
    command_to_start = "nohup /usr/local/bin/remotedebugger > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    command_to_get_pid = "pidof remotedebugger"
    pid = run_shell_command(command_to_get_pid)
    assert pid != "", "remotedebugger process did not start"

def test_remote_debugger_rbus_subscription():
    sleep(2)
    SUBSCRIBE = "SUCCESS: RBUS Event Subscribe for RRD done!"
    assert SUBSCRIBE in grep_rrdlogs(SUBSCRIBE)

def test_remote_debugger_waiting_for_events():
    sleep(2)
    EVENT_MSG = "Waiting for for TR69/RBUS Events."
    assert EVENT_MSG in grep_rrdlogs(EVENT_MSG)

    remove_logfile()
    kill_rrd()
