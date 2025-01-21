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

#ifndef _RRDRUNCMDTHREAD_H_
#define _RRDRUNCMDTHREAD_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include <unistd.h>
#include "rrdCommon.h"

#if !defined(GTEST_ENABLE)
#define SHELL_CMD "/bin/sh -c"
#define SYSTEMD_RUN "systemd-run -r --unit=remote_debugger"
#define SERVICE_TYPE "--service-type=oneshot"
#define SYSTEMD_STOP "systemctl stop remote_debugger"
#define SYSTEMD_RESET "systemctl reset-failed remote_debugger"
#define JOURNAL_CMD "journalctl -a -u remote_debugger"
#define RRD_OUTPUT_DIR "/tmp/rrd/"
#else
#define SHELL_CMD "/bin/sh -c"
#define SYSTEMD_STOP "echo"
#define SYSTEMD_RESET "echo"
#define SYSTEMD_RUN "echo"
#define SERVICE_TYPE "echo"
#define JOURNAL_CMD "echo"
#define RRD_OUTPUT_DIR "./"
#endif
#define RRD_OUTPUT_FILE "debug_outputs.txt"
#define BUF_LEN_256 256

/*Public Function*/
void initCache(void);
cacheData* createCache( char *pkgData, char *issueTypeData);
void print_items(cacheData *node);
void append_item(char *pkgData, char *issueTypeData);
void remove_item(cacheData *cache);
void freecacheDataCacheNode(cacheData **node);
cacheData* findPresentInCache(char *pkgData);
bool executeCommands(issueData *cmdinfo);
void removeQuotes(char* str);
void copyDebugLogDestFile(FILE *source, FILE *destination);

#ifdef __cplusplus
}
#endif

#endif
