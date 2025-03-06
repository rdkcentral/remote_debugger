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

Feature: Remote Debugger runs only one instance

  Scenario: Remote debugger exits if another instance is invoked
    Given the RemoteDebugger is not already running
    When the RemoteDebugger binary is invoked
    Then the RemoteDebugger should be started
    And when the RemoteDebugger is attempted to be started again
    Then the RemoteDebugger should not start another instance
