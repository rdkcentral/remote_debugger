from time import sleep
from helper_functions import *

def test_check_remotedebugger_is_starting():
    kill_rrd()
    remove_logfile()
    print("Starting remotedebugger process")
    command_to_start = "nohup /usr/local/bin/remotedebugger > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    command_to_get_pid = "pidof remotedebugger"
    pid = run_shell_command(command_to_get_pid)
    assert pid != "", "remotedebugger process did not start"

def test_second_remotedebugger_instance_is_not_started():
    command_to_get_pid = "pidof remotedebugger"
    pid1 = run_shell_command(command_to_get_pid)

    if is_remotedebugger_running():
        print("remotedebugger process is already running")
    else:
        command_to_start = "nohup /usr/local/bin/remotedebugger > /dev/null 2>&1 &"
        run_shell_silent(command_to_start)
        sleep(2)

    pid2 = run_shell_command(command_to_get_pid)
    assert pid1 == pid2, "A second instance of remotedebugger was started."

def test_tear_down():
    command_to_stop = "kill -9 `pidof remotedebugger`"
    run_shell_command(command_to_stop)
    command_to_get_pid = "pidof remotedebugger"
    remove_logfile()
    pid = run_shell_command(command_to_get_pid)
    assert pid == ""
