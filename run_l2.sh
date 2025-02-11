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
STATIC_PROFILE_DIR="/etc/rrd"
OUTPUT_DIR="/tmp/rrd"
LIB_DIR="/lib/rdk"
mkdir -p "$RESULT_DIR"
mkdir -p "$OUTPUT_DIR"
mkdir -p "$STATIC_PROFILE_DIR"
mkdir -p "$LIB_DIR"

touch  /etc/include.properties
echo "RDK_PATH=/lib/rdk" > /etc/include.properties

touch  /etc/dcm.properties
echo "LOG_SERVER=logs.xcal.tv" > /etc/dcm.properties
echo "HTTP_UPLOAD_LINK=https://ssr.ccp.xcal.tv/cgi-bin/S3.cgi" >> /etc/dcm.properties

touch  /bin/timestamp
echo "#!/bin/sh" > /bin/timestamp
echo "date -u +'%Y-%m-%dT%H:%M:%S.%3NZ'" > /bin/timestamp
chmod -R 777 /bin/timestamp

cp remote_debugger.json /etc/rrd/
rm -rf /opt/logs/remotedebugger.log*
cp scripts/uploadRRDLogs.sh /lib/rdk/uploadRRDLogs.sh
chmod -R 777 /lib/rdk/uploadRRDLogs.sh
rm -rf /tmp/rrd/*

# Run L2 Test cases
pytest  --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_single_instance.json test/functional-tests/tests/test_rrd_single_instance.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_start_control.json test/functional-tests/tests/test_rrd_start_control.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_start_subscribe_and_wait.json test/functional-tests/tests/test_rrd_start_subscribe_and_wait.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_static_profile_report.json test/functional-tests/tests/test_rrd_static_profile_report.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_corrupted_static_profile_report.json test/functional-tests/tests/test_rrd_corrupted_static_profile_report.py
cp remote_debugger.json /etc/rrd/
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_harmfull_static_profile_report.json test/functional-tests/tests/test_rrd_harmfull_static_profile_report.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_static_profile_category_report.json test/functional-tests/tests/test_rrd_static_profile_category_report.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_empty_event.json test/functional-tests/tests/test_rrd_empty_event.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_static_profile_missing_command_report.json test/functional-tests/tests/test_rrd_static_profile_missing_command_report.py
