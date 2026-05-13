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

Feature: Remote Debugger Static Profile Report With Overlength Suffix

  Scenario: Verify remote debugger process is running
    Given the remote debugger process is not running
    When I start the remote debugger process
    Then the remote debugger process should be running

  Scenario: Send RFC event with overlength suffixed issue type
    Given the remote debugger is running
    When I trigger the event "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType" with value "Device.Info_ab1bghjhfhk"
    Then the event for RRD_SET_ISSUE_EVENT should be received
    And the logs should contain "SUCCESS: Message sending Done"
    And the logs should be seen with "SUCCESS: Message Reception Done"
    And the remotedebugger request payload should include "MSG=Device.Info_ab1bghjhfhk"

  Scenario: Validate suffix discard and upload flow for overlength suffix
    Given the remotedebugger received the overlength suffixed issue type
    When remotedebugger parses "/etc/rrd/remote_debugger.json"
    Then remotedebugger should execute debug commands for static profile issue "Device.Info"
    And remotedebugger should log suffix-length discard with configured max length
    And the timestamped runtime service should start successfully
    And the timestamped runtime journalctl collection should succeed
    And the timestamped runtime service should stop successfully
    And remotedebugger should generate archive filename with discarded suffix
    And remotedebugger should invoke upload with the generated archive

  Scenario: Verify upload result for overlength suffix report
    When I check the upload status in the logs
    Then the upload script execution should be successful
    And the debug information report upload should be successful
