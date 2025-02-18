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
#define RRD_PROCESS_NAME "remotedebugger"
#define RRD_RBUS_TIMEOUT 60

#ifdef IARMBUS_SUPPORT
/*Enum for IARM Events*/
typedef enum _RemoteDebugger_EventId_t {
        IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE = 0,
        IARM_BUS_RDK_REMOTE_DEBUGGER_WEBCFGDATA,
        IARM_BUS_RDK_REMOTE_DEBUGGER_MAX_EVENT
} IARM_Bus_RemoteDebugger_EventId_t;
#endif

/*Event Handler Function*/
#if !defined(GTEST_ENABLE)
void _remoteDebuggerEventHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription);
void _remoteDebuggerWebCfgDataEventHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription);
void _rdmDownloadEventHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription)
#endif
#ifdef IARMBUS_SUPPORT
int RRD_IARM_subscribe(void);
int RRD_IARM_unsubscribe(void);
void _rdmManagerEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
void _pwrManagerEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
#endif
void RRD_data_buff_deAlloc(data_buf *sbuf);;
void RRDMsgDeliver(int msgqid, data_buf *sbuf);
int RRD_subscribe(void);
int RRD_unsubscribe(void);

#ifdef __cplusplus
}
#endif

#endif
