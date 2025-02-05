import subprocess

def run_shell_command(command):
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    return result.stdout.strip()

# Get RFC parameter value using rbuscli
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
    parameter = "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable"
    value = get_rfc_parameter(parameter)
    value = "false"

    if value == "true":
        start_daemon()
        pid = run_shell_command("pidof remotedebugger")
        assert pid != ""
    else:
        stop_daemon()
        pid = run_shell_command("pidof remotedebugger")
        assert pid == ""

