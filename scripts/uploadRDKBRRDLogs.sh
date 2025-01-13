#!/bin/sh
##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
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
# Source Default Variables
. /etc/include.properties
source $RDK_PATH/utils.sh
# Check for valid CLI args
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 UPLOADDIR ISSUETYPE"
    exit 1
fi
# Initialize the variables
RDK_LOGGER_PATH="/rdklogger"
MAC=`getMacAddressOnly`
TIMESTAMP=`date "+%Y-%m-%d-%H-%M-%S%p"`
RRD_LOG_FILE="$LOG_PATH/remote-debugger.log.0"
ISSUETYPE=`echo $2 | tr '[a-z]' '[A-Z]'`
RRD_LOG_PATH="$1"
RRD_LOG_DIR="/tmp/rrd/"
UPLOAD_DEBUG_FILE="${MAC}_${ISSUETYPE}_${TIMESTAMP}_RRD_DEBUG_LOGS.tgz"
source $RDK_LOGGER_PATH/logUpload_default_params.sh
# Logging Format
uploadLog()
{
    echo "`/bin/timestamp`: $0: $*" >> $RRD_LOG_FILE
}
getTFTPServer()
{
    if [ "$1" != "" ];then
        logserver=`grep -i $1 $RDK_LOGGER_PATH/dcmlogservers.txt | cut -f2 -d"|"`
        echo $logserver
    fi
}
BUILD_TYPE=`getBuildType`
SERVER=`getTFTPServer $BUILD_TYPE`
###################################
#  REMOTE DEBUGGER MAIN FUNCTION  #
###################################
uploadLog "Executing uploadRRDLogs.sh script to upload Debug info of ISSUETYPE=$ISSUETYPE"
uploadLog "Checking $RRD_LOG_PATH size and contents"
if [ -d $RRD_LOG_PATH ] && [ "$(ls -A $RRD_LOG_PATH)" ]; then
    cd $RRD_LOG_DIR
    uploadLog "Creating $UPLOAD_DEBUG_FILE tarfile from Debug Commands output"
    tar -zcf $UPLOAD_DEBUG_FILE -C $RRD_LOG_PATH . >> $RRD_LOG_FILE 2>&1
    uploadLog "Invoking uploadRDKBLogs.sh script to upload $UPLOAD_DEBUG_FILE file"
    $RDK_LOGGER_PATH/uploadRDKBLogs.sh $SERVER "HTTP" $URL "false" "" $RRD_LOG_DIR "false"
    retval=$?
    if [ $retval -ne 200 ];then
        uploadLog "RRD $ISSUETYPE Debug Information Report upload Failed!!!"
        rm -rf $UPLOAD_DEBUG_FILE $RRD_LOG_PATH
    else
        uploadLog "RRD $ISSUETYPE Debug Information Report upload Success"
        uploadLog "Removing uploaded report $UPLOAD_DEBUG_FILE"
        rm -rf $UPLOAD_DEBUG_FILE $RRD_LOG_PATH
    fi
else
    uploadLog "$RRD_LOG_PATH is Empty, Exiting!!!"
fi
