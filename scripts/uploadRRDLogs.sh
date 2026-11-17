#!/bin/sh
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

# Source Default Variables
. /etc/include.properties
. /etc/device.properties
. $RDK_PATH/utils.sh

# Check for valid CLI args
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 UPLOADDIR ISSUETYPE"
    exit 1
fi

# Initialize the variables
OUTFILE='/tmp/DCMSettings.conf'
MAC=`getMacAddressOnly`
TIMESTAMP=`date "+%Y-%m-%d-%H-%M-%S%p"`
RRD_LOG_FILE="$LOG_PATH/remote-debugger.log"
ISSUETYPE=`echo $2 | tr '[a-z]' '[A-Z]'`
RRD_LOG_PATH="$1"
RRD_REPORT_NAME=${RRD_LOG_PATH#/tmp/rrd/}
case "$RRD_LOG_PATH" in
    /tmp/rrd/*) ;;
    *)
        echo "Refusing unexpected report path: $RRD_LOG_PATH" >&2
        exit 1
        ;;
esac
case "$RRD_REPORT_NAME" in
    ""|.|..|*/*)
        echo "Refusing unexpected report directory name: $RRD_REPORT_NAME" >&2
        exit 1
        ;;
esac
RRD_LOG_DIR="/tmp/rrd/"
UPLOAD_DEBUG_FILE="${MAC}_${ISSUETYPE}_${TIMESTAMP}_RRD_DEBUG_LOGS.tgz"
case "$UPLOAD_DEBUG_FILE" in
    ""|.|..|*/*)
        echo "Refusing unexpected archive filename: $UPLOAD_DEBUG_FILE" >&2
        exit 1
        ;;
esac
UPLOAD_PROTOCOL="HTTP"

# Logging Format
uploadLog()
{
    echo "`/bin/timestamp`: $0: $*" >> $RRD_LOG_FILE
}

# Keep descriptor 9 open so validation and parsing use the same inode.
openTrustedConfig()
{
    if [ ! -f "$1" ] || [ -L "$1" ]; then
        uploadLog "Refusing missing, non-regular or symlinked config: $1"
        return 1
    fi
    exec 9< "$1"
    configMetadata=$(stat -Lc '%u %a' "/proc/$$/fd/9") || {
        uploadLog "Cannot inspect config: $1"
        exec 9<&-
        return 1
    }
    configOwner=${configMetadata% *}
    configMode=${configMetadata#* }
    case "$configMode" in
        ""|*[!0-7]*)
            uploadLog "Invalid config permissions: $1"
            exec 9<&-
            return 1
            ;;
    esac
    if [ ! -f "/proc/$$/fd/9" ] || [ "$configOwner" != 0 ] ||
       [ $((0$configMode & 022)) -ne 0 ]; then
        uploadLog "Refusing config without trusted ownership/permissions: $1"
        exec 9<&-
        return 1
    fi
}

uploadRRDLogsSTB()
{
    PARAM_LOG_SERVER=$1
    PARAM_UPLOAD_PROTOCOL=$2
    PARAM_HTTP_UPLOAD_LINK=$3
    PARAM_RRD_LOG_DIR=$4
    PARAM_UPLOAD_DEBUG_FILE=$5
    max_attempts=10
    attempt=1
    result=1

    cd "$PARAM_RRD_LOG_DIR" || return 1
    while [ $attempt -le $max_attempts ]; do
        if [ ! -f /tmp/.log-upload.pid ]; then
            # Call the LogUploadSTB.sh to upload RRD Logs
            sh $RDK_PATH/uploadSTBLogs.sh "$PARAM_LOG_SERVER" 1 1 0 "$PARAM_UPLOAD_PROTOCOL" "$PARAM_HTTP_UPLOAD_LINK" 0 1 "$PARAM_UPLOAD_DEBUG_FILE"
            result=$?
            break
        else
            # Condition is not met
            echo "One instance already running for uploadSTBLogs.sh. So Sleeping for 60 seconds..."
            sleep 60
            attempt=$((attempt + 1))
        fi
    done
    return $result
}

if [ "$BUILD_TYPE" != "prod" ] && [ -f /opt/dcm.properties ]; then
    uploadLog "Configurable service end-points will not be used for $BUILD_TYPE Builds due to overriden /opt/dcm.properties!!!"
else
    if [ -f /usr/bin/tr181 ]; then
        #Fetch Upload LogUrl information
        uploadLog "Using Log Server Url from RFC parameter:Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.LogServerUrl..."
        LOG_SERVER=$(tr181 -g Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.LogServerUrl 2>&1)
        if [ -z "$HTTP_UPLOAD_LINK" ]; then
            uploadLog "'LogUploadSettings:UploadRepository:URL' is not found in DCMSettings.conf, Reading from RFC"
            UPLOAD_HTTPLINK_URL=$(tr181 -g Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.SsrUrl 2>&1)
            if [ ! -z "$UPLOAD_HTTPLINK_URL" ]; then
                HTTP_UPLOAD_LINK=${UPLOAD_HTTPLINK_URL}/cgi-bin/S3.cgi
            fi
        fi
    fi
    #Fetch Upload HttpLink information
    uploadLog "Using Upload HttpLink from DCMSettings.conf..."
    if [ -e "$OUTFILE" ] || [ -L "$OUTFILE" ]; then
        openTrustedConfig "$OUTFILE" || exit 1
        dcmSettings=$(cat <&9)
        configReadStatus=$?
        exec 9<&-
        if [ "$configReadStatus" -ne 0 ]; then
            uploadLog "Cannot read DCM settings"
            exit 1
        fi
        HTTP_UPLOAD_LINK=$(printf '%s\n' "$dcmSettings" | grep 'LogUploadSettings:UploadRepository:URL' | cut -d '=' -f2 | sed 's/^"//' | sed 's/"$//')
        #Fetch Upload Protocol information
        uploadLog "Using Upload Protocol from DCMSettings.conf..."
        UPLOAD_PROTOCOL=$(printf '%s\n' "$dcmSettings" | grep 'LogUploadSettings:UploadRepository:uploadProtocol' | cut -d '=' -f2 | sed 's/^"//' | sed 's/"$//')
        if [ -z "$UPLOAD_PROTOCOL" ]; then
            uploadLog "urn:settings:LogUploadSettings:Protocol' is not found in DCMSettings.conf"
            UPLOAD_PROTOCOL="HTTP"
        fi
    fi
fi

if [ -z $LOG_SERVER ] || [ -z $HTTP_UPLOAD_LINK ]; then
    echo "DCM params read using RFC/tr181 is empty..!!!"
    if [ "$BUILD_TYPE" != "prod" ] && [ -f /opt/dcm.properties ]; then
        dcmProperties=/opt/dcm.properties
    else
        dcmProperties=/etc/dcm.properties
    fi
    openTrustedConfig "$dcmProperties" || exit 1
    . "/proc/$$/fd/9"
    exec 9<&-
fi
###################################
#  REMOTE DEBUGGER MAIN FUNCTION  #
###################################
uploadLog "Executing remote_debugger.sh Script to upload Debug info of ISSUETYPE=$ISSUETYPE"
uploadLog "Checking $RRD_LOG_PATH size and contents"
if [ -d "$RRD_LOG_PATH" ] && [ -n "$(ls -A "$RRD_LOG_PATH")" ]; then
    cd "$RRD_LOG_DIR" || exit 1
    if [ "$ISSUETYPE" = "LOGUPLOAD_ENABLE" ]; then
        uploadLog "Check and upload live device logs for the issuetype"
        mv -- RRD_LIVE_LOGS.tar.gz "$RRD_LOG_PATH"
    fi
    uploadLog "Creating $UPLOAD_DEBUG_FILE tarfile from Debug Commands output"
    tar -zcf "$UPLOAD_DEBUG_FILE" -C "$RRD_LOG_PATH" . >> "$RRD_LOG_FILE" 2>&1
    uploadLog "Invoking uploadSTBLogs script to upload $UPLOAD_DEBUG_FILE file"
    uploadLog "$RDK_PATH/uploadSTBLogs.sh $LOG_SERVER 1 1 0 $UPLOAD_PROTOCOL $HTTP_UPLOAD_LINK 0 1 $UPLOAD_DEBUG_FILE"
    uploadRRDLogsSTB "$LOG_SERVER" "$UPLOAD_PROTOCOL" "$HTTP_UPLOAD_LINK" "$RRD_LOG_DIR" "$UPLOAD_DEBUG_FILE"
    retval=$?
    if [ $retval -ne 0 ];then
        uploadLog "RRD $ISSUETYPE Debug Information Report upload Failed!!!"
        rm -rf -- "$UPLOAD_DEBUG_FILE" "$RRD_LOG_PATH"
    else
        uploadLog "RRD $ISSUETYPE Debug Information Report upload Success"
        uploadLog "Removing uploaded report $UPLOAD_DEBUG_FILE"
        rm -rf -- "$UPLOAD_DEBUG_FILE" "$RRD_LOG_PATH"
    fi
else
    uploadLog "$RRD_LOG_PATH is Empty, Exiting!!!"
fi
