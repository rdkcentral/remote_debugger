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
#include "libIARM.h"
#include "libIBus.h"
#include "libIARMCore.h"
#endif

#define IARM_BUS_RDK_REMOTE_DEBUGGER_NAME "REMOTE_DEBUGGER"

/*Enum for IARM Events*/
typedef enum _RemoteDebugger_EventId_t {
        IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE = 0,
        IARM_BUS_RDK_REMOTE_DEBUGGER_WEBCFGDATA,
        IARM_BUS_RDK_REMOTE_DEBUGGER_MAX_EVENT
} IARM_Bus_RemoteDebugger_EventId_t;

/*IARM Event Handler Function*/
void _remoteDebuggerEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
void _rdmManagerEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
void _pwrManagerEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
void _remoteDebuggerWebCfgDataEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
void RRD_data_buff_deAlloc(data_buf *sbuf);
IARM_Result_t RRD_subscribe(void);
IARM_Result_t RRD_unsubscribe(void);

#ifdef __cplusplus
}
#endif

#endif
