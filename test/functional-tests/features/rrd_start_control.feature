####################################################################################
# If not stated otherwise in this file or this component's Licenses.txt file the
# following copyright and licenses apply:
#
# Copyright 2024 RDK Management
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
####################################################################################


Feature: Remote Debugger Enable or Disable with WebPA Value

  Scenario: Remote Debugger Starts when Enabled
    Given RFC Value for Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable is enabled
    When the remotedebugger is started check the value of the RFC parameter
    And  the RDKRemoteDebugger Enable value is true
    Then the remotedebugger should be started and running as daemon

  Scenario: Remote Debugger Starts when Disabled
    Given RFC Value for Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable is disabled
    When the remotedebugger is started check the value of the RFC parameter
    And  the RDKRemoteDebugger Enable value is false
    Then the remotedebugger must be stopped and process should not be running
