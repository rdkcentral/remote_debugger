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
Feature: Remote Debugger C API Upload Orchestration

  Scenario: Validate rrd_upload_orchestrate C API with valid parameters
    Given the remote debugger is configured
    And test log files are created in the upload directory
    When I call rrd_upload_orchestrate with valid upload directory and issue type
    Then the C API should return success code 0
    And logs should contain "Executing binary to upload Debug info of ISSUETYPE"
    And logs should contain "Logging ready"
    And logs should contain "Configuration loaded"
    And logs should contain "MAC:" for MAC address
    And logs should contain "Timestamp:" for timestamp generation
    And logs should contain "size and contents" for directory size check
    And logs should contain "Log directory validated and prepared"
    And logs should contain "Issue type sanitized"
    And logs should contain "Archive filename:" with the generated filename
    And logs should contain "Creating" for tarfile creation
    And logs should contain "tarfile from Debug Commands output"
    And logs should contain "Invoking uploadSTBLogs binary to upload"
    And logs should contain "uploadSTBLogs parameters - server:"
    And the archive should be created with correct naming format
    And the archive should contain all log files from the directory
    And the upload should be triggered successfully

  Scenario: Test rrd_upload_orchestrate with NULL upload directory
    Given the remote debugger is configured
    When I call rrd_upload_orchestrate with NULL upload directory
    Then the C API should return error code 1
    And error logs should contain "Invalid parameters"

  Scenario: Test rrd_upload_orchestrate with NULL issue type
    Given the remote debugger is configured
    And test log files are created in the upload directory
    When I call rrd_upload_orchestrate with NULL issue type
    Then the C API should return error code 1
    And error logs should contain "Invalid parameters"

  Scenario: Test rrd_upload_orchestrate with empty upload directory
    Given the remote debugger is configured
    And the upload directory is empty
    When I call rrd_upload_orchestrate with the empty directory
    Then the C API should return error code 6
    And error logs should contain "Invalid or empty upload directory"

  Scenario: Test rrd_upload_orchestrate with non-existent directory
    Given the remote debugger is configured
    When I call rrd_upload_orchestrate with non-existent directory
    Then the C API should return error code 6
    And error logs should contain "Directory does not exist"

  Scenario: Test rrd_upload_orchestrate configuration loading
    Given the remote debugger configuration files exist
    When I call rrd_upload_orchestrate with valid parameters
    Then configuration should be loaded from /etc/include.properties
    And RFC parameters should be queried via tr181 if available
    And DCM settings should be parsed from /tmp/DCMSettings.conf
    And fallback to dcm.properties should work if needed
    And logs should show final configuration values

  Scenario: Test rrd_upload_orchestrate MAC address retrieval
    Given the system has a valid MAC address
    When I call rrd_upload_orchestrate with valid parameters
    Then MAC address should be retrieved successfully
    And logs should show "MAC address obtained"
    And archive filename should include the MAC address

  Scenario: Test rrd_upload_orchestrate timestamp generation
    Given the system time is available
    When I call rrd_upload_orchestrate with valid parameters
    Then timestamp should be generated in format YYYY-MM-DD-HH-MM-SSAM/PM
    And logs should show "Timestamp generated"
    And archive filename should include the timestamp

  Scenario: Test rrd_upload_orchestrate issue type sanitization
    Given the remote debugger is configured
    And test log files are created
    When I call rrd_upload_orchestrate with issue type "test.issue-type"
    Then issue type should be sanitized to "TEST_ISSUE_TYPE"
    And logs should show issue type conversion
    And archive filename should use sanitized issue type

  Scenario: Test rrd_upload_orchestrate archive creation
    Given the remote debugger is configured
    And test log files are created in the upload directory
    When I call rrd_upload_orchestrate with valid parameters
    Then a tar.gz archive should be created
    And the archive should be in valid gzip format
    And the archive should contain POSIX tar headers
    And all files from upload directory should be in archive

  Scenario: Test rrd_upload_orchestrate upload execution
    Given the remote debugger is configured
    And a test archive is ready for upload
    And upload server is reachable
    When I call rrd_upload_orchestrate with valid parameters
    Then upload lock should be checked before upload
    And upload parameters should be prepared correctly
    And uploadstblogs_run should be called with rrd_flag=true
    And logs should show "Upload completed successfully"

  Scenario: Test rrd_upload_orchestrate cleanup after success
    Given the remote debugger is configured
    And successful upload has completed
    When I call rrd_upload_orchestrate with valid parameters
    Then logs should contain "Debug Information Report upload Success"
    And logs should contain "Removing uploaded report"
    And logs should contain "Exit" for orchestration exit
    And the archive file should be cleaned up
    And temporary files should be removed

  Scenario: Test rrd_upload_orchestrate cleanup after upload failure
    Given the remote debugger is configured
    And upload will fail
    When I call rrd_upload_orchestrate with valid parameters
    Then logs should contain "Debug Information Report upload Failed"
    And the archive file should still be cleaned up
    And error code 11 should be returned

  Scenario: Test uploadDebugoutput wrapper function
    Given the remote debugger is running
    And test log files are created
    When I call uploadDebugoutput with valid parameters
    Then issue name should be normalized (dots to underscores)
    And rrd_upload_orchestrate should be called
    And success should be logged for successful upload
    And failure should be logged for failed upload

  Scenario: Test concurrent upload lock handling
    Given the remote debugger is configured
    And another upload is in progress (lock file exists)
    When I call rrd_upload_orchestrate with valid parameters
    Then upload lock should be detected
    And API should wait for lock to clear
    And upload should proceed after lock clears
    Or timeout error should be returned if lock persists

  Scenario: Test rrd_upload_orchestrate with LOGUPLOAD_ENABLE issue type
    Given the remote debugger is configured
    And live logs are available
    When I call rrd_upload_orchestrate with issue type "LOGUPLOAD_ENABLE"
    Then live logs should be handled specially
    And logs should be prepared for upload
    And archive should include live log data

  Scenario: Test remote debugger end-to-end with RFC trigger
    Given the remote debugger process is running
    And the issue type RFC is reset to empty string
    When I set RFC parameter "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType" to test issue string
    Then logs should contain "Received event for RRD_SET_ISSUE_EVENT"
    And logs should contain "SUCCESS: Message sending Done"
    And logs should contain "SUCCESS: Message Reception Done"
    And logs should contain "MSG=" followed by the issue string
    And logs should contain "Start Reading JSON File... /etc/rrd/remote_debugger.json"
    And logs should contain "Json File parse Success... /etc/rrd/remote_debugger.json"
    And logs should contain "Issue Data Node:" with node and subnode information
    And logs should contain "Creating Directory"
    And logs should contain "Found valid Commands"
    And logs should contain "Adding Details of Debug commands to Output File"
    And logs should contain "Starting remote_debugger_" service success message
    And logs should contain "journalctl remote_debugger_" service success message
    And logs should contain "Sleeping with timeout"
    And logs should contain "Stopping remote_debugger_" service message
    And logs should contain "Starting Upload Debug output via API"
    And the output directory should contain generated files

  Scenario: Test upload report validation with success path
    Given the remote debugger has completed upload orchestration
    When upload completes successfully
    Then logs should contain "Debug Information Report upload Success"
    And logs should contain "Removing uploaded report"
    And logs should contain "Exit" for orchestration completion
    And logs should contain "Logging ready" from orchestration initialization
    And logs should contain "Configuration loaded" from orchestration setup

  Scenario: Test upload report validation with failure path
    Given the remote debugger has completed upload orchestration
    When upload fails
    Then logs should contain "Debug Information Report upload Failed"
    And logs should contain "Logging ready" from orchestration initialization
    And logs should contain "Configuration loaded" from orchestration setup
    And cleanup should still occur

  Scenario: Test upload report with legacy log compatibility
    Given the remote debugger is using legacy logging
    When upload completes
    Then logs should contain either "RRD Upload Script Execution Success" or "RRD Upload Script Execution Failure"
    And upload orchestration logs should confirm execution

  Scenario: Test rrd_upload_orchestrate error propagation
    Given the remote debugger is configured
    When configuration loading fails
    Then error code 3 should be returned
    When MAC address retrieval fails
    Then error code 4 should be returned
    When timestamp generation fails
    Then error code 5 should be returned
    When directory validation fails
    Then error code 6 should be returned
    When log preparation fails
    Then error code 7 should be returned
    When issue type sanitization fails
    Then error code 8 should be returned
    When archive filename generation fails
    Then error code 9 should be returned
    When archive creation fails
    Then error code 10 should be returned
    When upload execution fails
    Then error code 11 should be returned
