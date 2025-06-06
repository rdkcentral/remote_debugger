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

#!/bin/bash

# Step 1: Create a JSON file
json_file="/media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json"
echo '{
  "Test": {
    "TestRun3": {
      "Commands": "cat /version.txt;uptime;rm -rf;cat /tmp/.deviceDetails.cache",
      "Timeout": 10
    },
    "TestRun2": {
      "Commands": "cat /version.txt;uptime;/proc/version;cat /proc/buddyinfo;cat /proc/meminfo;cat /tmp/.deviceDetails.cache",
      "Timeout": 10
    },
    "TestRun1": {
      "Commands": "cat /version.txt;uptime;cat /proc/buddyinfo;cat /proc/meminfo;cat /tmp/.deviceDetails.cache",
      "Timeout": 10
    }
  }
}' > $json_file
