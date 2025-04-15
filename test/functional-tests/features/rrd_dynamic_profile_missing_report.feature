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

Feature: Remote Debugger Missing Issuetype

  Scenario: Verify remote debugger process is running
    Given the remote debugger process is not running
    When I start the remote debugger process
    Then the remote debugger process should be running

  Scenario: Send WebPA event for Issuetype Test and verify logs
    Given the remote debugger is running
    When I trigger the event "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType"
    Then the logs should contain "SUCCESS: Message sending Done"
    Then the logs should be seen with "SUCCESS: Message Reception Done"
    And the issuetype request should match between Send and Receive

  Scenario: Verify the Issuetype is not found in static profile
    Given the remote debugger received the message from RBUS command
    When the remotedebugger static json profile is present
    Then remotedebugger should read the Json file
    And remotedebugger logs should contain the Json File Parse Success
    And remotedebugger should log as the Issue requested is not found in the profile
    
  Scenario: Verify the Issuetype is not found in dynamic path
    Given the remote debugger issuetype is missing in static profile
    When the remotedebugger read the json file form the dynamic path
    Then remotedebugger json read and parse should be failed
    And remotedebugger should request RDM to download the package
