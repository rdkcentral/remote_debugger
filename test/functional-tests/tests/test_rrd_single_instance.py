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
    pid1 = run_shell_command(command_to_get_pid).strip().split()

    if is_remotedebugger_running():
        print("remotedebugger process is already running")
    else:
        command_to_start = "nohup /usr/local/bin/remotedebugger > /dev/null 2>&1 &"
        run_shell_silent(command_to_start)
        sleep(2)

    pid2 = run_shell_command(command_to_get_pid).strip().split()

    # Ensure no new process was spawned (set of PIDs should be the same)
    assert set(pid1) == set(pid2), f"A second instance of remotedebugger was started: {pid2}"


def test_tear_down():
    command_to_stop = "kill -9 `pidof remotedebugger`"
    run_shell_command(command_to_stop)
    command_to_get_pid = "pidof remotedebugger"
    remove_logfile()
    remove_outdir_contents(OUTPUT_DIR)
    pid = run_shell_command(command_to_get_pid)
    assert pid == ""
