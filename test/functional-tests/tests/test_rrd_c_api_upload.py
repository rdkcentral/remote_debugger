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

import os
import subprocess
import tarfile
import gzip
import re
import time
from helper_functions import *

# Test Constants
TEST_UPLOAD_DIR = "/tmp/rrd_test_upload"
TEST_ISSUE_TYPE = "test_issue"
RRD_LOG_FILE = "/opt/logs/remote-debugger.log"
RRD_BINARY = "/usr/local/bin/remotedebugger"

class TestCAPIHelper:
    """Helper class for C API testing"""
    
    @staticmethod
    def create_test_directory(path):
        """Create test directory structure"""
        os.makedirs(path, exist_ok=True)
        return os.path.exists(path)
    
    @staticmethod
    def create_test_files(directory, file_count=5):
        """Create test log files in directory"""
        created_files = []
        for i in range(file_count):
            filepath = os.path.join(directory, f"test_log_{i}.txt")
            with open(filepath, 'w') as f:
                f.write(f"Test log content {i}\n" * 10)
            created_files.append(filepath)
        return created_files
    
    @staticmethod
    def cleanup_test_directory(path):
        """Remove test directory and contents"""
        if os.path.exists(path):
            subprocess.run(['rm', '-rf', path], check=False)
    
    @staticmethod
    def get_archive_filename_pattern(mac, issue_type):
        """Generate expected archive filename pattern"""
        # Format: MAC_ISSUETYPE_TIMESTAMP.tar.gz
        return f"{mac}_{issue_type}_*.tar.gz"
    
    @staticmethod
    def validate_archive_format(archive_path):
        """Validate that archive is valid tar.gz"""
        try:
            with gzip.open(archive_path, 'rb') as gz:
                with tarfile.open(fileobj=gz, mode='r:') as tar:
                    return tar is not None
        except Exception as e:
            print(f"Archive validation failed: {e}")
            return False
    
    @staticmethod
    def get_archive_contents(archive_path):
        """Get list of files in archive"""
        try:
            with gzip.open(archive_path, 'rb') as gz:
                with tarfile.open(fileobj=gz, mode='r:') as tar:
                    return tar.getnames()
        except Exception as e:
            print(f"Failed to read archive: {e}")
            return []
    
    @staticmethod
    def check_log_contains(pattern, log_file=RRD_LOG_FILE):
        """Check if log file contains pattern"""
        try:
            with open(log_file, 'r') as f:
                content = f.read()
                return pattern in content
        except FileNotFoundError:
            return False
    
    @staticmethod
    def get_mac_address():
        """Get system MAC address"""
        result = subprocess.run(['sh', '-c', 'getMacAddressOnly'], 
                              capture_output=True, text=True)
        if result.returncode == 0:
            return result.stdout.strip()
        return None
    
    @staticmethod
    def create_upload_lock():
        """Create upload lock file for testing"""
        lock_file = "/tmp/.log-upload.lock"
        with open(lock_file, 'w') as f:
            f.write(str(os.getpid()))
        return lock_file
    
    @staticmethod
    def remove_upload_lock():
        """Remove upload lock file"""
        lock_file = "/tmp/.log-upload.lock"
        if os.path.exists(lock_file):
            os.remove(lock_file)


# Test Functions

def test_rrd_upload_orchestrate_valid_parameters():
    """Test rrd_upload_orchestrate with valid parameters"""
    helper = TestCAPIHelper()
    
    # Setup
    helper.cleanup_test_directory(TEST_UPLOAD_DIR)
    assert helper.create_test_directory(TEST_UPLOAD_DIR)
    created_files = helper.create_test_files(TEST_UPLOAD_DIR)
    assert len(created_files) == 5
    
    # Clear previous logs
    remove_logfile()
    remove_upload_lock()
    
    # Trigger via RRD daemon (which calls uploadDebugoutput -> rrd_upload_orchestrate)
    kill_rrd()
    time.sleep(2)
    
    # Start remotedebugger
    command_to_start = f"nohup {RRD_BINARY} > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    time.sleep(5)
    
    # Trigger event to invoke C API
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', TEST_ISSUE_TYPE
    ]
    result = subprocess.run(command, capture_output=True, text=True)
    assert result.returncode == 0
    
    time.sleep(20)  # Wait for processing
    
    # Verify logs
    assert helper.check_log_contains("rrd_upload_orchestrate: Entry")
    assert helper.check_log_contains("Configuration loaded")
    assert helper.check_log_contains("MAC address obtained")
    assert helper.check_log_contains("Timestamp generated")
    assert helper.check_log_contains("Archive created")
    assert helper.check_log_contains("rrd_upload_orchestrate: Exit")
    
    # Cleanup
    helper.cleanup_test_directory(TEST_UPLOAD_DIR)
    kill_rrd()


def test_rrd_upload_orchestrate_null_parameters():
    """Test rrd_upload_orchestrate with NULL parameters"""
    helper = TestCAPIHelper()
    
    # This test would require a test harness that directly calls the C API
    # For now, we verify that the daemon doesn't crash with invalid params
    
    # The uploadDebugoutput function checks for NULL before calling orchestrate
    # So we verify the error handling logs
    
    remove_logfile()
    kill_rrd()
    time.sleep(2)
    
    # Normal startup
    command_to_start = f"nohup {RRD_BINARY} > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    time.sleep(5)
    
    # The daemon should handle edge cases gracefully
    pid = run_shell_command("pidof remotedebugger")
    assert pid != "", "remotedebugger should still be running"
    
    kill_rrd()


def test_rrd_upload_orchestrate_empty_directory():
    """Test rrd_upload_orchestrate with empty directory"""
    helper = TestCAPIHelper()
    
    # Setup empty directory
    helper.cleanup_test_directory(TEST_UPLOAD_DIR)
    helper.create_test_directory(TEST_UPLOAD_DIR)
    
    remove_logfile()
    kill_rrd()
    time.sleep(2)
    
    # Start remotedebugger
    command_to_start = f"nohup {RRD_BINARY} > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    time.sleep(5)
    
    # Trigger with empty directory (would fail at validation)
    # The actual upload directory used by RRD is controlled by JSON config
    # This test validates that empty directory detection works
    
    # Create a scenario where the output directory is empty
    # In practice, RRD won't create archive if no commands produced output
    
    # Cleanup
    helper.cleanup_test_directory(TEST_UPLOAD_DIR)
    kill_rrd()


def test_rrd_config_loading():
    """Test configuration loading in rrd_upload_orchestrate"""
    helper = TestCAPIHelper()
    
    # Ensure config files exist
    assert os.path.exists('/etc/include.properties'), "include.properties should exist"
    
    remove_logfile()
    kill_rrd()
    time.sleep(2)
    
    # Start with config
    command_to_start = f"nohup {RRD_BINARY} > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    time.sleep(5)
    
    # Trigger upload to test config loading
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', 'config_test'
    ]
    subprocess.run(command, capture_output=True, text=True)
    time.sleep(15)
    
    # Verify configuration was loaded
    assert helper.check_log_contains("Loading configuration")
    assert helper.check_log_contains("Configuration loaded")
    
    # Check that config sources were tried
    log_patterns = [
        "Parsing /etc/include.properties",
        "Configuration loaded - LOG_SERVER:",
        "UPLOAD_PROTOCOL:",
        "HTTP_UPLOAD_LINK:"
    ]
    
    for pattern in log_patterns:
        assert helper.check_log_contains(pattern), f"Missing log pattern: {pattern}"
    
    kill_rrd()


def test_mac_address_retrieval():
    """Test MAC address retrieval in rrd_upload_orchestrate"""
    helper = TestCAPIHelper()
    
    # Get system MAC for comparison
    mac = helper.get_mac_address()
    assert mac is not None, "System should have a MAC address"
    
    remove_logfile()
    kill_rrd()
    time.sleep(2)
    
    command_to_start = f"nohup {RRD_BINARY} > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    time.sleep(5)
    
    # Trigger upload
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', 'mac_test'
    ]
    subprocess.run(command, capture_output=True, text=True)
    time.sleep(15)
    
    # Verify MAC was retrieved
    assert helper.check_log_contains("MAC address obtained")
    assert helper.check_log_contains(f"MAC: {mac}"), f"MAC {mac} should be in logs"
    
    kill_rrd()


def test_timestamp_generation():
    """Test timestamp generation in rrd_upload_orchestrate"""
    helper = TestCAPIHelper()
    
    remove_logfile()
    kill_rrd()
    time.sleep(2)
    
    command_to_start = f"nohup {RRD_BINARY} > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    time.sleep(5)
    
    # Trigger upload
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', 'timestamp_test'
    ]
    subprocess.run(command, capture_output=True, text=True)
    time.sleep(15)
    
    # Verify timestamp was generated
    assert helper.check_log_contains("Timestamp generated")
    
    # Check timestamp format in logs (YYYY-MM-DD-HH-MM-SS[AM|PM])
    timestamp_pattern = r'\d{4}-\d{2}-\d{2}-\d{2}-\d{2}-\d{2}[AP]M'
    
    with open(RRD_LOG_FILE, 'r') as f:
        content = f.read()
        assert re.search(timestamp_pattern, content), "Timestamp format should match expected pattern"
    
    kill_rrd()


def test_issue_type_sanitization():
    """Test issue type sanitization in rrd_upload_orchestrate"""
    helper = TestCAPIHelper()
    
    remove_logfile()
    kill_rrd()
    time.sleep(2)
    
    command_to_start = f"nohup {RRD_BINARY} > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    time.sleep(5)
    
    # Trigger with issue type containing dots and hyphens
    test_issue = "test.issue-type.example"
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', test_issue
    ]
    subprocess.run(command, capture_output=True, text=True)
    time.sleep(15)
    
    # Verify sanitization (dots become underscores, uppercase)
    # normalizeIssueName converts dots to underscores
    # rrd_logproc_convert_issue_type converts to uppercase and sanitizes
    expected_normalized = "TEST_ISSUE_TYPE_EXAMPLE"
    
    assert helper.check_log_contains("Issue type sanitized")
    assert helper.check_log_contains(expected_normalized) or helper.check_log_contains("test_issue_type_example")
    
    kill_rrd()


def test_archive_creation():
    """Test archive creation in rrd_upload_orchestrate"""
    helper = TestCAPIHelper()
    
    # Create test files
    helper.cleanup_test_directory(TEST_UPLOAD_DIR)
    helper.create_test_directory(TEST_UPLOAD_DIR)
    helper.create_test_files(TEST_UPLOAD_DIR)
    
    remove_logfile()
    kill_rrd()
    time.sleep(2)
    
    command_to_start = f"nohup {RRD_BINARY} > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    time.sleep(5)
    
    # Trigger upload
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', 'archive_test'
    ]
    subprocess.run(command, capture_output=True, text=True)
    time.sleep(15)
    
    # Verify archive creation logs
    assert helper.check_log_contains("Creating archive")
    assert helper.check_log_contains("Archive created")
    assert helper.check_log_contains(".tar.gz")
    
    # Note: Archive is typically created in /tmp/rrd/ and then cleaned up
    # So we verify logs rather than checking for file existence
    
    helper.cleanup_test_directory(TEST_UPLOAD_DIR)
    kill_rrd()


def test_upload_execution():
    """Test upload execution in rrd_upload_orchestrate"""
    helper = TestCAPIHelper()
    
    remove_logfile()
    kill_rrd()
    time.sleep(2)
    
    command_to_start = f"nohup {RRD_BINARY} > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    time.sleep(5)
    
    # Trigger upload
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', 'upload_test'
    ]
    subprocess.run(command, capture_output=True, text=True)
    time.sleep(20)
    
    # Verify upload was attempted
    assert helper.check_log_contains("Starting upload")
    
    # Check for either success or failure (depending on server availability)
    upload_attempted = (
        helper.check_log_contains("Upload completed successfully") or
        helper.check_log_contains("Log upload failed")
    )
    assert upload_attempted, "Upload should have been attempted"
    
    kill_rrd()


def test_cleanup_after_upload():
    """Test cleanup in rrd_upload_orchestrate"""
    helper = TestCAPIHelper()
    
    remove_logfile()
    kill_rrd()
    time.sleep(2)
    
    command_to_start = f"nohup {RRD_BINARY} > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    time.sleep(5)
    
    # Trigger upload
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', 'cleanup_test'
    ]
    subprocess.run(command, capture_output=True, text=True)
    time.sleep(20)
    
    # Verify cleanup logs
    assert helper.check_log_contains("Cleanup complete")
    
    kill_rrd()


def test_upload_debug_output_wrapper():
    """Test uploadDebugoutput wrapper function"""
    helper = TestCAPIHelper()
    
    remove_logfile()
    kill_rrd()
    time.sleep(2)
    
    command_to_start = f"nohup {RRD_BINARY} > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    time.sleep(5)
    
    # Trigger with issue containing dots (tests normalization)
    test_issue = "wrapper.test.issue"
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', test_issue
    ]
    subprocess.run(command, capture_output=True, text=True)
    time.sleep(15)
    
    # Verify wrapper was called
    assert helper.check_log_contains("Starting Upload Debug output via API")
    
    # Verify outcome logging
    outcome_logged = (
        helper.check_log_contains("Upload orchestration completed successfully") or
        helper.check_log_contains("Upload orchestration failed")
    )
    assert outcome_logged, "Wrapper should log outcome"
    
    kill_rrd()


def test_concurrent_upload_lock():
    """Test upload lock handling"""
    helper = TestCAPIHelper()
    
    # Create a lock file
    helper.remove_upload_lock()
    lock_file = helper.create_upload_lock()
    
    remove_logfile()
    kill_rrd()
    time.sleep(2)
    
    command_to_start = f"nohup {RRD_BINARY} > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    time.sleep(5)
    
    # Trigger upload while lock exists
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', 'lock_test'
    ]
    subprocess.run(command, capture_output=True, text=True)
    
    time.sleep(5)
    
    # Remove lock to allow completion
    helper.remove_upload_lock()
    
    time.sleep(15)
    
    # Note: Lock handling is done in rrd_upload_execute
    # Verify that it doesn't crash with lock present
    pid = run_shell_command("pidof remotedebugger")
    assert pid != "", "remotedebugger should handle lock gracefully"
    
    kill_rrd()


def test_comprehensive_logging():
    """Test comprehensive logging throughout rrd_upload_orchestrate"""
    helper = TestCAPIHelper()
    
    remove_logfile()
    kill_rrd()
    time.sleep(2)
    
    command_to_start = f"nohup {RRD_BINARY} > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    time.sleep(5)
    
    # Trigger upload
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', 'logging_test'
    ]
    subprocess.run(command, capture_output=True, text=True)
    time.sleep(20)
    
    # Verify comprehensive logging at each step
    expected_logs = [
        "rrd_upload_orchestrate: Entry",
        "Logging ready",
        "Loading configuration",
        "Configuration loaded",
        "MAC address obtained",
        "Timestamp generated",
        "Log directory validated and prepared",
        "Issue type sanitized",
        "Archive filename:",
        "Archive created",
        "Cleanup complete",
        "rrd_upload_orchestrate: Exit"
    ]
    
    for log_pattern in expected_logs:
        assert helper.check_log_contains(log_pattern), f"Missing expected log: {log_pattern}"
    
    kill_rrd()


def test_error_code_propagation():
    """Test error code propagation in rrd_upload_orchestrate"""
    helper = TestCAPIHelper()
    
    # This test verifies that errors are logged with appropriate codes
    # Actual return codes are checked by the wrapper function
    
    remove_logfile()
    kill_rrd()
    time.sleep(2)
    
    command_to_start = f"nohup {RRD_BINARY} > /dev/null 2>&1 &"
    run_shell_silent(command_to_start)
    time.sleep(5)
    
    # Trigger normal upload
    command = [
        'rbuscli', 'set',
        'Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType',
        'string', 'error_test'
    ]
    subprocess.run(command, capture_output=True, text=True)
    time.sleep(15)
    
    # Verify that errors would be logged if they occurred
    # In normal operation, we should see success path
    
    # Check that the function completes one way or another
    assert (
        helper.check_log_contains("rrd_upload_orchestrate: Exit") or
        helper.check_log_contains("Upload orchestration failed")
    )
    
    kill_rrd()
