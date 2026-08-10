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
# Run L2 integration tests with gcov/lcov coverage instrumentation.
#
# Prerequisites (inside the test container):
#   lcov, genhtml, gcc with --coverage support
#
# Usage:
#   sh run_l2_coverage.sh
#
# Output:
#   /tmp/l2_coverage/html/index.html  — browsable HTML coverage report
#   /tmp/l2_coverage/coverage.info    — lcov tracefile for CI upload

set -e

WORKDIR="$(pwd)"
INSTALL_DIR=/usr/local
RESULT_DIR="/tmp/l2_test_report"
COV_DIR="/tmp/l2_coverage"
STATIC_PROFILE_DIR="/etc/rrd"
OUTPUT_DIR="/tmp/rrd"
LIB_DIR="/lib/rdk"

# Tell helper_functions.py to use SIGTERM so gcov flushes before exit.
export RRD_COVERAGE_MODE=1

# ── Directories ──────────────────────────────────────────────────────────────
mkdir -p "$RESULT_DIR"
mkdir -p "$OUTPUT_DIR"
mkdir -p "$STATIC_PROFILE_DIR"
mkdir -p "$LIB_DIR"
mkdir -p "$COV_DIR"
mkdir -p /media/apps/RDK-RRD-Test/etc/rrd

# ── System fixtures (mirrors run_l2.sh) ──────────────────────────────────────
touch /media/apps/RDK-RRD-Test/etc/rrd/remote_debugger.json
echo "AA:BB:CC:DD:EE:FF" >> /tmp/.estb_mac

apt-get remove -y systemd || true
apt-get update && apt-get install -y tcpdump lcov

echo "LOG_PATH=/opt/logs" >> /etc/include.properties
cp remote_debugger.json "$STATIC_PROFILE_DIR/remote_debugger.json"

cp scripts/uploadRRDLogs.sh "$LIB_DIR/uploadRRDLogs.sh"
chmod 777 "$LIB_DIR/uploadRRDLogs.sh"
sed -i 's/remote-debugger\.log/remotedebugger\.log\.0/g' "$LIB_DIR/uploadRRDLogs.sh"

cp test/functional-tests/tests/uploadSTBLogs.sh "$LIB_DIR/uploadSTBLogs.sh"
chmod 777 "$LIB_DIR/uploadSTBLogs.sh"

cp scripts/systemd-run /usr/local/bin/systemd-run
chmod 777 /usr/local/bin/systemd-run
ln -sf /usr/local/bin/systemd-run /usr/bin/systemd-run

touch /usr/local/bin/systemctl
chmod 777 /usr/local/bin/systemctl
ln -sf /usr/local/bin/systemctl /usr/bin/systemctl

touch /usr/local/bin/journalctl
chmod 777 /usr/local/bin/journalctl
ln -sf /usr/local/bin/journalctl /usr/bin/journalctl

rm -rf /tmp/rrd/*
rm -rf /opt/logs/remotedebugger.log*

# ── Coverage build ────────────────────────────────────────────────────────────
autoreconf -i
autoupdate
./configure --prefix="${INSTALL_DIR}" --enable-iarmbusSupport=yes

# Append --coverage to the same CFLAGS/LDFLAGS used in cov_build.sh.
make remotedebugger_CFLAGS="-I/usr/include/cjson -I/usr/local/include/wdmp-c \
 -I/usr/local/include/rbus -I/usr/local/include -I./unittest/mocks \
 -I/usr/local/include/trower-base64 -DIARMBUS_SUPPORT -DUSECOV -DUSE_L2_SUPPORT \
 --coverage" \
  remotedebugger_LDFLAGS="-L/usr/local/lib -lrdkloggers -lcjson -lrfcapi -lrbus \
 -lmsgpackc -lsecure_wrapper -lwebconfig_framework -lIARMBus -ltr181api \
 -L/usr/local/lib/x86_64-linux-gnu -ltrower-base64 -L/usr/lib/x86_64-linux-gnu \
 --coverage"
make install

# ── lcov baseline (all lines counted as zero-hit) ────────────────────────────
lcov --zerocounters --directory "$WORKDIR/src"
lcov --capture --initial \
     --directory "$WORKDIR/src" \
     --output-file "$COV_DIR/coverage_base.info" \
     --rc lcov_branch_coverage=1

# ── L2 test suite ─────────────────────────────────────────────────────────────
run_test() {
    pytest --json-report --json-report-summary \
           --json-report-file "$RESULT_DIR/$1.json" \
           "test/functional-tests/tests/$2" || true
}

run_test rrd_dynamic_profile_missing_report      test_rrd_dynamic_profile_missing_report.py
run_test test_category                           test_rrd_dynamic_subcategory_report.py
run_test rrd_append                              test_rrd_append_report.py
run_test rrd_dynamic_profile_harmful_report      test_rrd_dynamic_profile_harmful_report.py
cp remote_debugger.json "$STATIC_PROFILE_DIR/"
run_test rrd_dynamic_profile_report              test_rrd_dynamic_profile_report.py
run_test rrd_append_dynamic_profile_static_notfound test_rrd_append_dynamic_profile_static_notfound.py
run_test rrd_single_instance                     test_rrd_single_instance.py
run_test rrd_start_control                       test_rrd_start_control.py
run_test rrd_start_subscribe_and_wait            test_rrd_start_subscribe_and_wait.py
run_test rrd_static_profile_report               test_rrd_static_profile_report.py
run_test rrd_static_profile_report_with_suffix   test_rrd_static_profile_report_with_suffix.py
run_test rrd_static_profile_report_with_suffix_negative test_rrd_static_profile_report_with_suffix_negative_case.py
run_test rrd_corrupted_static_profile_report     test_rrd_corrupted_static_profile_report.py
cp remote_debugger.json "$STATIC_PROFILE_DIR/"
run_test rrd_harmful_static_profile_report       test_rrd_harmful_command_static_report.py
run_test rrd_static_profile_category_report      test_rrd_static_profile_category_report.py

# ── Capture post-test coverage ────────────────────────────────────────────────
lcov --capture \
     --directory "$WORKDIR/src" \
     --output-file "$COV_DIR/coverage_test.info" \
     --rc lcov_branch_coverage=1

# ── Merge baseline + test, then strip system headers ─────────────────────────
lcov --add-tracefile "$COV_DIR/coverage_base.info" \
     --add-tracefile "$COV_DIR/coverage_test.info" \
     --output-file "$COV_DIR/coverage_merged.info" \
     --rc lcov_branch_coverage=1

lcov --remove "$COV_DIR/coverage_merged.info" \
     '/usr/*' \
     --output-file "$COV_DIR/coverage.info" \
     --rc lcov_branch_coverage=1

# ── HTML report ───────────────────────────────────────────────────────────────
genhtml "$COV_DIR/coverage.info" \
        --output-directory "$COV_DIR/html" \
        --title "Remote Debugger L2 Coverage" \
        --branch-coverage \
        --legend

echo ""
echo "Coverage report : $COV_DIR/html/index.html"
echo "lcov tracefile  : $COV_DIR/coverage.info"
