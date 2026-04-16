##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2026 RDK Management
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

"""
Simple integration tests for RDK Remote Debugger Profile Data RBUS functionality.
Demonstrates usage of rbuscli commands for setProfileData and getProfileData parameters.
"""

import subprocess
import json
import time
from helper_functions import *

def test_rrd_profile_data_rbuscli_basic():
    """Basic test of rbuscli commands for RRD profile data."""
    
    kill_rrd()
    remove_logfile()
    
    # RBUS parameter names - exactly as defined in the HLD
    set_param = "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.setProfileData"
    get_param = "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.getProfileData"
    
    def run_rbuscli_cmd(cmd):
        """Execute rbuscli command and return result."""
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=30)
        return result.stdout.strip(), result.stderr.strip(), result.returncode
    
    # Test Case 1: Set profile data to "all" and get all categories
    print("Test Case 1: Setting profile data to 'all'")
    stdout, stderr, rc = run_rbuscli_cmd(f'rbuscli set "all" {set_param}')
    print(f"Set command result: stdout='{stdout}', stderr='{stderr}', rc={rc}")
    assert rc == 0, f"rbuscli set 'all' failed: {stderr}"
    
    time.sleep(2)  # Allow processing time
    
    print("Getting profile data after setting to 'all'")
    stdout, stderr, rc = run_rbuscli_cmd(f'rbuscli get {get_param}')
    print(f"Get command result: stdout='{stdout}', stderr='{stderr}', rc={rc}")
    assert rc == 0, f"rbuscli get failed: {stderr}"
    
    if stdout:
        try:
            data = json.loads(stdout)
            print(f"Parsed JSON data: {data}")
            assert isinstance(data, (list, dict)), "Expected JSON array or object"
        except json.JSONDecodeError:
            print(f"Warning: Could not parse JSON response: {stdout}")
    
    # Test Case 2: Set profile data to specific category  
    print("\nTest Case 2: Setting profile data to 'Device'")
    stdout, stderr, rc = run_rbuscli_cmd(f'rbuscli set "Device" {set_param}')
    print(f"Set command result: stdout='{stdout}', stderr='{stderr}', rc={rc}")
    assert rc == 0, f"rbuscli set 'Device' failed: {stderr}"
    
    time.sleep(2)  # Allow processing time
    
    print("Getting profile data after setting to 'Device'")
    stdout, stderr, rc = run_rbuscli_cmd(f'rbuscli get {get_param}')
    print(f"Get command result: stdout='{stdout}', stderr='{stderr}', rc={rc}")
    assert rc == 0, f"rbuscli get failed: {stderr}"
    
    if stdout:
        try:
            data = json.loads(stdout)
            print(f"Parsed JSON data for Device: {data}")
            assert isinstance(data, (list, dict)), "Expected JSON array or object"
        except json.JSONDecodeError:
            print(f"Warning: Could not parse JSON response: {stdout}")
    
    # Test Case 3: Set profile data to another category
    print("\nTest Case 3: Setting profile data to 'Process'")
    stdout, stderr, rc = run_rbuscli_cmd(f'rbuscli set "Process" {set_param}')
    print(f"Set command result: stdout='{stdout}', stderr='{stderr}', rc={rc}")
    assert rc == 0, f"rbuscli set 'Process' failed: {stderr}"
    
    time.sleep(2)  # Allow processing time
    
    print("Getting profile data after setting to 'Process'")
    stdout, stderr, rc = run_rbuscli_cmd(f'rbuscli get {get_param}')
    print(f"Get command result: stdout='{stdout}', stderr='{stderr}', rc={rc}")
    assert rc == 0, f"rbuscli get failed: {stderr}"
    
    if stdout:
        try:
            data = json.loads(stdout)
            print(f"Parsed JSON data for Process: {data}")
        except json.JSONDecodeError:
            print(f"Warning: Could not parse JSON response: {stdout}")
    
    # Clean up
    remove_logfile()
    kill_rrd()
    
    print("All rbuscli tests completed successfully!")

def test_rrd_profile_data_error_cases():
    """Test error cases for RRD profile data rbuscli commands."""
    
    set_param = "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.setProfileData"
    get_param = "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.getProfileData"
    
    def run_rbuscli_cmd(cmd):
        """Execute rbuscli command and return result."""
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=30)
        return result.stdout.strip(), result.stderr.strip(), result.returncode
    
    # Test Case 1: Set to non-existent category
    print("Error Test 1: Setting to non-existent category")
    stdout, stderr, rc = run_rbuscli_cmd(f'rbuscli set "NonExistentCategory" {set_param}')
    print(f"Set result: stdout='{stdout}', stderr='{stderr}', rc={rc}")
    
    # Check if this is an invalid arguments error or successful set
    if "Invalid arguments" in stdout:
        print("rbuscli returned invalid arguments error (expected for malformed command)")
    else:
        # Should succeed (system accepts any string)
        assert rc == 0, f"rbuscli set should accept any string: {stderr}"
        
        time.sleep(1)
        
        # Get should handle gracefully
        print("Getting data for non-existent category")
        stdout, stderr, rc = run_rbuscli_cmd(f'rbuscli get {get_param}')
        print(f"Get result: stdout='{stdout}', stderr='{stderr}', rc={rc}")
        # Should return success with empty array or fallback
        assert rc == 0, f"rbuscli get should handle invalid category: {stderr}"
    
    # Test Case 2: Empty string
    print("\nError Test 2: Setting to empty string")
    stdout, stderr, rc = run_rbuscli_cmd(f'rbuscli set "" {set_param}')
    print(f"Set result: stdout='{stdout}', stderr='{stderr}', rc={rc}")
    
    # Check result - might be invalid arguments or successful empty string set
    if "Invalid arguments" not in stdout:
        # If it's a successful set, test the get operation
        time.sleep(1)
        stdout, stderr, rc = run_rbuscli_cmd(f'rbuscli get {get_param}')
        print(f"Get result after empty set: stdout='{stdout}', stderr='{stderr}', rc={rc}")
        assert rc == 0, f"rbuscli get should handle empty category: {stderr}"
    
    # Test Case 3: Try to set with wrong parameter syntax
    print("\nError Test 3: Wrong parameter syntax")
    stdout, stderr, rc = run_rbuscli_cmd('rbuscli set "test" WrongParameter')
    print(f"Wrong param result: stdout='{stdout}', stderr='{stderr}', rc={rc}")
    
    # rbuscli returns 0 but outputs "Invalid arguments" for wrong parameters
    assert "Invalid arguments" in stdout or rc != 0, f"rbuscli should indicate error for wrong parameter, got: {stdout}"
    
    print("Error case tests completed!")

if __name__ == "__main__":
    test_rrd_profile_data_rbuscli_basic()
    test_rrd_profile_data_error_cases()
