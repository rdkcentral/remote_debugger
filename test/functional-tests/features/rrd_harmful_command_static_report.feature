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

Feature: Remote Debugger Harmful Commands Report

  Scenario: Verify remote debugger process is running
    Given the remote debugger process is not running
    When I start the remote debugger process
    Then the remote debugger process should be running

  Scenario: Send WebPA event for Issuetype Command.Harm
    Given the remote debugger is running
    When I trigger the event "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType"
    Then the logs should contain "SUCCESS: Message sending Done"
    Then the logs should be seen with "SUCCESS: Message Reception Done"
    And the issuetype request should match between Send and Receive
    When the remotedebugger received the message from webPA event
    Then remotedebugger should read the Json file
    And remotedebugger logs should contain the Json File Parse Success

  Scenario: Check for harmfull commands and abort
    Given remote debugger parse the static json profile successfully
    When the issue node and subnode are present in the profile
    Then the logs should contain the "Reading Command and Timeout information for Debug Issue"
    Then the remote debugger should read the Sanity Check list from profile
    And the remotedebugger should perform sanity check on issue commands
    Given the remote debugger profile has the harmfull commands
    When the issue command and the sanity commands are matched
    Then the remote debugger should exit the processing of commands
    And Abort the commmand execution and skip report upload

