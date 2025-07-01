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

RESULT_DIR="/tmp/l2_test_report"
STATIC_PROFILE_DIR="/etc/rrd"
OUTPUT_DIR="/tmp/rrd"
LIB_DIR="/lib/rdk"

mkdir -p "$RESULT_DIR"
mkdir -p "$OUTPUT_DIR"
mkdir -p "$STATIC_PROFILE_DIR"
mkdir -p "$LIB_DIR"
mkdir -p /media/apps/RDK-RRD-Test/etc/rrd

touch /media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json
/usr/local/bin/tr69hostif -c /etc/mgrlist.conf -d /etc/debug.ini -p 10999 -s 11999
apt-get remove systemd
apt-get update && apt-get install -y tcpdump

echo "LOG_PATH=/opt/logs" >> /etc/include.properties
cp remote_debugger.json /etc/rrd/remote_debugger.json
cp scripts/uploadRRDLogs.sh /lib/rdk/uploadRRDLogs.sh
chmod -R 777 /lib/rdk/uploadRRDLogs.sh
sed -i 's/remote-debugger\.log/remotedebugger\.log\.0/g' /lib/rdk/uploadRRDLogs.sh

cp test/functional-tests/tests/uploadSTBLogs.sh /lib/rdk/uploadSTBLogs.sh
chmod -R 777 /lib/rdk/uploadRRDLogs.sh

cp scripts/systemd-run /usr/local/bin/systemd-run
chmod -R 777 /usr/local/bin/systemd-run
ln -s /usr/local/bin/systemd-run /usr/bin/systemd-run

touch /usr/local/bin/systemctl
chmod -R 777 /usr/local/bin/systemctl
ln -s /usr/local/bin/systemctl /usr/bin/systemctl

touch /usr/local/bin/journalctl
chmod -R 777 /usr/local/bin/journalctl
ln -s /usr/local/bin/journalctl /usr/bin/journalctl

rm -rf /tmp/rrd/*
rm -rf /opt/logs/remotedebugger.log*

# Run L2 Test cases
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_neg.json test/functional-tests/tests/test_rrd_negative.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_dynamic_profile_missing_report.json test/functional-tests/tests/test_rrd_dynamic_profile_missing_report.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_append.json test/functional-tests/tests/test_append.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_dynamic_profile_harmful_report.json test/functional-tests/tests/test_rrd_dynamic_profile_harmful_report.py
cp remote_debugger.json /etc/rrd/
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_dynamic_profile_report.json test/functional-tests/tests/test_rrd_dynamic_profile_report.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_append_dynamic_profile_static_notfound.json test/functional-tests/tests/test_rrd_append_dynamic_profile_static_notfound.py
pytest  --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_single_instance.json test/functional-tests/tests/test_rrd_single_instance.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_start_control.json test/functional-tests/tests/test_rrd_start_control.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_start_subscribe_and_wait.json test/functional-tests/tests/test_rrd_start_subscribe_and_wait.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_static_profile_report.json test/functional-tests/tests/test_rrd_static_profile_report.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_corrupted_static_profile_report.json test/functional-tests/tests/test_rrd_corrupted_static_profile_report.py
cp remote_debugger.json /etc/rrd/
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_harmfull_static_profile_report.json test/functional-tests/tests/test_rrd_harmful_command_static_report.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_static_profile_category_report.json test/functional-tests/tests/test_rrd_static_profile_category_report.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_empty_event.json test/functional-tests/tests/test_rrd_empty_issuetype_event.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_static_profile_missing_command_report.json test/functional-tests/tests/test_rrd_static_profile_missing_command_report.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_background_cmd_static_profile_report.json test/functional-tests/tests/test_rrd_background_cmd_static_profile_report.py
pytest --json-report --json-report-summary --json-report-file $RESULT_DIR/rrd_debug_report_upload.json test/functional-tests/tests/test_rrd_debug_report_upload.py
