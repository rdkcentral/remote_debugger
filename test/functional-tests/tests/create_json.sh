#!/bin/bash

# Step 1: Create a JSON file
json_file="/media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json"
echo '{
  "Test": {
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
