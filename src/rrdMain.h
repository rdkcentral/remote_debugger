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

#ifndef _RRDMAIN_H_
#define _RRDMAIN_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/msg.h>
#include <pthread.h>
#include "rrdCommon.h"
#if !defined(GTEST_ENABLE)
#ifdef IARMBUS_SUPPORT
#include "rfcapi.h"
#else
#include "syscfg/syscfg.h"
#endif
#endif

#define DEBUG_INI_FILE "/etc/debug.ini"
#define RRD_RFC "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable"

/* Global Variables*/
#if !defined(GTEST_ENABLE)
int msqid = 0;
key_t key = 1234;

void * RRDEventThreadFunc(void *arg);
void RRDStoreDeviceInfo(devicePropertiesData *devPropData);
void removeSpecialChar(char *str);
bool isRRDEnabled(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
