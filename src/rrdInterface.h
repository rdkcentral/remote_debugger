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

#ifndef _RRDIARM_H_
#define _RRDIARM_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/msg.h>
#include "rrdCommon.h"
#if !defined(GTEST_ENABLE)
#include "rbus.h"
#include <cjson/cJSON.h>
#ifdef IARMBUS_SUPPORT
#include "libIARM.h"
#include "libIBus.h"
#include "libIARMCore.h"
#endif
#endif

#define RDK_REMOTE_DEBUGGER_NAME "REMOTE_DEBUGGER"
#define REMOTE_DEBUGGER_RBUS_HANDLE_NAME "rdkRrdRbus"
#define RRD_WEBCFG_FORCE_SYNC "Device.X_RDK_WebConfig.ForceSync"
#define RRD_SET_ISSUE_EVENT "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType"
#define RRD_WEBCFG_ISSUE_EVENT "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.WebCfgData"
#define RDM_DOWNLOAD_EVENT "Device.DeviceInfo.X_RDKCENTRAL-COM_RDKDownloadManager.DownloadStatus"
#define RRD_PROCESS_NAME "remotedebugger"
#define RRD_RBUS_TIMEOUT 60

// RDK Remote Debugger profile data parameter definitions
#define RRD_SET_PROFILE_EVENT "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.setProfileData"
#define RRD_GET_PROFILE_EVENT "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.getProfileData"
#define RRD_PROFILE_CATEGORY_FILE "/tmp/rrd_profile_category"

/*Enum for IARM Events*/
typedef enum _RemoteDebugger_EventId_t {
        IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE = 0,
        IARM_BUS_RDK_REMOTE_DEBUGGER_WEBCFGDATA,
        IARM_BUS_RDK_REMOTE_DEBUGGER_MAX_EVENT
} IARM_Bus_RemoteDebugger_EventId_t;

/*Event Handler Function*/
#if !defined(GTEST_ENABLE)
void _remoteDebuggerEventHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription);
void _remoteDebuggerWebCfgDataEventHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription);
void _rdmDownloadEventHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription);

// Helper functions for profile data processing
bool has_direct_commands(cJSON *category);
char* read_profile_json_file(const char* filename, long* file_size);
char* get_all_categories_json(cJSON* json);
char* get_specific_category_json(cJSON* json, const char* category_name);
rbusError_t set_rbus_response(rbusProperty_t prop, const char* json_str);
#endif
#if defined(IARMBUS_SUPPORT) || defined(GTEST_ENABLE)
int RRD_IARM_subscribe(void);
int RRD_IARM_unsubscribe(void);
void _rdmManagerEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
#if defined(PWRMGR_PLUGIN)
void _pwrManagerEventHandler(const PowerController_PowerState_t currentState,
    const PowerController_PowerState_t newState, void* userdata);
#else
void _pwrManagerEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
#endif
#endif
void RRD_data_buff_deAlloc(data_buf *sbuf);
void RRDMsgDeliver(int msgqid, data_buf *sbuf);
int RRD_subscribe(void);
int RRD_unsubscribe(void);
rbusError_t rrd_SetHandler(rbusHandle_t handle, rbusProperty_t property, rbusSetHandlerOptions_t* opts);
rbusError_t rrd_GetHandler(rbusHandle_t handle, rbusProperty_t prop, rbusGetHandlerOptions_t* opts);

#ifdef __cplusplus
}
#endif

#endif
