/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef _RRDCOMMON_H_
#define _RRDCOMMON_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#if !defined(GTEST_ENABLE)
#include "rdk_debug.h"
#ifdef IARMBUS_SUPPORT
#include "rdmMgr.h"
#include "pwrMgr.h"
#include "tr181api.h"
#endif
#endif

#ifndef ENABLE_WEBCFG_FEATURE
    #define ENABLE_WEBCFG_FEATURE
#endif
#define RRD_JSON_FILE "/etc/rrd/remote_debugger.json"
#if !defined(GTEST_ENABLE)
#define RRD_DEVICE_PROP_FILE "/etc/device.properties"
#else
#define RRD_DEVICE_PROP_FILE "UTJson/device.properties"
#endif
#define LOG_REMDEBUG "LOG.RDK.REMOTEDEBUGGER"
#define DEEP_SLEEP_STR "Test"
#define RDM_MGR_PKG_INST "Device.DeviceInfo.X_RDKCENTRAL-COM_RDKDownloadManager.InstallPackage"
#define RRD_MEDIA_APPS "/media/apps/"
#define RDM_PKG_PREFIX "RDK-RRD-"
#define RDM_PKG_SUFFIX ":1.0"

#ifndef RRD_PROFILE_LIST
#define RRD_DEVICE_PROFILE ""
#else
#define RRD_DEVICE_PROFILE RRD_PROFILE_LIST
#endif

#define BUF_LEN_128  128
#define APPEND_SUFFIX "_apnd"

/* Enum for Messages Queue*/
typedef enum {
     RRD_EVENT_MSG_DEFAULT = 0,
     RRD_EVENT_MSG_REQUEST,
} rrd_event_msg_type_et;

/* Enum for Events*/
typedef enum {
     DEFAULT = 0,
     EVENT_MSG,
     EVENT_WEBCFG_MSG,
     DEEPSLEEP_EVENT_MSG,
} message_type_et;

/* Enum for Messages Queue*/
typedef enum {
     RRD_DEEPSLEEP_INVALID_DEFAULT = 0,
     RRD_DEEPSLEEP_RDM_DOWNLOAD_PKG_INITIATE,
     RRD_DEEPSLEEP_RDM_PKG_INSTALL_COMPLETE,
} deepsleep_event_et;

/*Structure for Message Queue and JSon Parser*/
typedef struct mbuffer {
     message_type_et     mtype;
     char                *mdata;
     char                *jsonPath;
     bool                inDynamic;
     bool                appendMode;
     deepsleep_event_et  dsEvent;
} data_buf;

/*Structure for Message Header*/
typedef struct messageHeader {
    long  type;
    void  *mbody;
} msgRRDHdr;

/*Structure for Json Datar*/
typedef struct jsondata {
     char * rfcvalue;
     char * command;
     int  timeout;
} issueData;

/*Structure for Issue Node*/
typedef struct issueNode{
     char *Node;
     char *subNode;
} issueNodeData;

/*Structure for Cache Data*/
typedef struct cache {
     char *mdata;
     char *issueString;
     struct cache *next;
     struct cache *prev;
} cacheData;

/*Structure for Device Propertiesr*/
typedef struct deviceProperties {
	char *deviceType;
}devicePropertiesData;

int decodeWebCfgData(char *pString);
void pushIssueTypesToMsgQueue(char *issueTypeList, message_type_et sndtype);
void RRD_data_buff_init(data_buf *sbuf, message_type_et sndtype, deepsleep_event_et  deepSleepEvent);
void RRDRdmManagerDownloadRequest(issueNodeData *pissueStructNode, char *dynJSONPath, data_buf *rbuf, bool isDeepSleepAwakeEvent);
bool lookupRrdProfileList(const char *profile);
const char* getRrdProfileName(devicePropertiesData *devPropData);
bool checkAppendRequest(char *issuerequest);

#ifdef __cplusplus
}
#endif

#endif
