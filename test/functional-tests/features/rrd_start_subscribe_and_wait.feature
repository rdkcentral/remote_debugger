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

Feature: Remote Debugger Subscribe to RBUS events

  Scenario: Remote debugger should subscribe to events
    Given the remote debugger binary is invoked
    When the remote debugger is started
    Then the remote debuger should subscribe to rbus and wait for the events
    And the log file should contain "SUCCESS: RBUS Event Subscribe for RRD done!"
    And the log file should contain "Waiting for TR69/RBUS Events..."
