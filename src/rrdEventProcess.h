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

#ifndef _RRDEVENTPROCESS_H_
#define _RRDEVENTPROCESS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "rrdCommon.h"
#include <ctype.h>

void processIssueTypeEvent(data_buf *rbuf);
void processWebCfgTypeEvent(data_buf *rbuf);
issueData* processIssueTypeInStaticProfileappend(data_buf *rbuf, issueNodeData *pIssueNode);
issueData* processIssueTypeInDynamicProfileappend(data_buf *rbuf, issueNodeData *pIssueNode);
#ifdef __cplusplus
}
#endif

#endif
