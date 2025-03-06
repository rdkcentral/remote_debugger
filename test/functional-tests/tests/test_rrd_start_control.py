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
from helper_functions import *

def run_shell_command(command):
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    return result.stdout.strip()

def get_rfc_parameter(parameter):
    command = f"rbuscli get {parameter}"
    return run_shell_command(command)

def start_daemon():
    command = "nohup /usr/local/bin/remotedebugger > /dev/null 2>&1 &"
    run_shell_command(command)

def stop_daemon():
    command = "kill -9 `pidof remotedebugger`"
    run_shell_command(command)

def test_rdk_remote_debugger_enabled():
    kill_rrd()
    remove_logfile()    
    parameter = "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable"
    value = get_rfc_parameter(parameter)

    if value == "true":
        start_daemon()
        pid = run_shell_command("pidof remotedebugger")
        assert pid != ""
    else:
        stop_daemon()
        pid = run_shell_command("pidof remotedebugger")
        assert pid == ""

    remove_logfile()
    remove_outdir_contents(OUTPUT_DIR)
    kill_rrd()
