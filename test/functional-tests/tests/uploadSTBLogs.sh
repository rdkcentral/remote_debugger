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
#!/bin/sh

TFTP_SERVER=$1
FLAG=$2
DCM_FLAG=$3
UploadOnReboot=$4
UploadProtocol=$5
UploadHttpLink=$6
TriggerType=$7
RRD_FLAG=$8
RRD_UPLOADLOG_FILE=$9

uploadLog()
{
    echo "`/bin/timestamp`: $0: $*" >> /opt/logs/remotedebugger.log.0
}

if [ "$RRD_FLAG" -eq 1 ]; then
    RRD_DIR="/tmp/rrd/"
    UploadHttpLink="https://mockxconf:50054/rrdUploadFile"
    ret=`curl -k -F "file=@$RRD_DIR$RRD_UPLOADLOG_FILE" $UploadHttpLink --insecure -w "%{http_code}" -o /dev/null`
    if [ $? -eq 0 ]; then
        uploadLog "Curl command executed successfully."
        if [ "$ret" = "200" ];then
            uploadLog "Uploading Logs through HTTP Success..., HTTP response code: $ret"
            exit 0
        else
            uploadLog "Uploading Logs through HTTP Failed!!!, HTTP response code: $ret"
            exit 127
        fi
    else
        uploadLog "Curl command failed with return code $?."
        exit 127
    fi
fi
