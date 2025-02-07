#!/bin/sh
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

RESULT_DIR="/tmp/remotedebugger_test_report"
OUTPUT_DIR="/tmp/rrd"
LIB_DIR="/lib/rdk"
mkdir -p "$RESULT_DIR"
mkdir -p "$OUTPUT_DIR"
mkdir -p "$LIB_DIR"

cp remote_debugger.json /etc/rrd/
cp scripts/uploadRRDLogs.sh /lib/rdk/uploadRRDLogs.sh
rm -rf /opt/logs/remotedebugger.log.0

# Run L2 Test cases
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_single_instance.json test/functional-tests/tests/test_rrd_single_instance.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_start_control.json test/functional-tests/tests/test_rrd_start_control.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_start_subscribe_and_wait.json test/functional-tests/tests/test_rrd_start_subscribe_and_wait.py
