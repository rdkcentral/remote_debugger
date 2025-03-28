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

#ifndef _RRDJSONPARSER_H_
#define _RRDJSONPARSER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rrdCommon.h"
#include "cJSON.h"

#define DEFAULT_TIMEOUT 90
#define RFC_DELIM "."

typedef enum {
     RRD_CATEGORY = 0,
     RRD_TYPE
} rrd_nodes_t;

int getParamcount(char *str);
char* readJsonFile(char *jsonfile);
cJSON* readAndParseJSON(char *jsonFile);
void getIssueInfo(char *issuestr, issueNodeData *issue);
bool findIssueInParsedJSON(issueNodeData *issuestructNode, cJSON *jsoncfg);
void checkIssueNodeInfo(issueNodeData *issuestructNode, cJSON *jsoncfg, data_buf *buff, bool isDeepSleepAwakeEventValid, issueData *appendprofiledata);
bool invokeSanityandCommandExec(issueNodeData *issuestructNode, cJSON *jsoncfg,char *buf, bool deepSleepAwkEvnt);
issueData* getIssueCommandInfo(issueNodeData *issuestructNode, cJSON *jsoncfg,char *buf);
bool processAllDebugCommand(cJSON *jsoncfg, issueNodeData *issuestructNode, char *rfcbuf);
bool processAllDeepSleepAwkMetricsCommands(cJSON *jsoncfg, issueNodeData *issuestructNode, char *rfcbuf);

#ifdef __cplusplus
}
#endif

#endif
