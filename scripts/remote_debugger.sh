#!/bin/sh
##############################################################################
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2020 RDK Management
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
##############################################################################

. /etc/include.properties
. /etc/device.properties

REDIRECT_FILE="/dev/null"
TR181_BIN="/usr/bin/tr181 -g"
RRD_BIN="/usr/bin/remotedebugger"
TIMESTAMP=`date -u +%y%m%d-%H:%M:%S`
FILENAME=`basename $0`
PID="$$"
RRD_ENABLE_TR181_NAME="Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable"

ENABLE_RRD=`${TR181_BIN} ${RRD_ENABLE_TR181_NAME} 2>&1 > ${REDIRECT_FILE}`

if [ "x$ENABLE_RRD" != "xtrue" ]; then
    echo "${PID}: ${TIMESTAMP}: ${FILENAME}: RDK Remote Debugger Status is Disabled: $ENABLE_RRD"
    echo "${PID}: ${TIMESTAMP}: ${FILENAME}: Disable RDK Remote Debugger!!!"
    systemctl stop remote-debugger
    systemctl reset-failed  remote-debugger
else
    echo "${PID}: ${TIMESTAMP}: ${FILENAME}: RDK Remote Debugger Status is Enabled: $ENABLE_RRD"
    echo "${PID}: ${TIMESTAMP}: ${FILENAME}: Enable RDK Remote Debugger..."
    ${RRD_BIN} &
fi
