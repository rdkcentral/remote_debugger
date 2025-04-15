#!/bin/bash

# Step 1: Create a JSON file
json_file="/media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json"
echo '{
  "Test": {
    "TestRun5": {
      "Commands": "cat /version.txt;uptime;cat /tmp/.deviceDetails.cache",
      "Timeout": 10                                                                                                                      },
    "TestRun4": {
      "Commands": "cat /version.txt;uptime;cat /tmp/.deviceDetails.cache",
      "Timeout": 10                                                                                                                      },
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
mkdir -p /tmp/RDK-RRD-Test/etc/rrd/
cp $json_file /tmp/RDK-RRD-Test/etc/rrd/
