/*
 *
 * If not stated otherwise in this file or this component's Licenses.txt file the
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

#ifndef RRD_LOGPROC_H
#define RRD_LOGPROC_H

#include <stddef.h>

int rrd_logproc_validate_source(const char *source_dir);
int rrd_logproc_prepare_logs(const char *source_dir, const char *issue_type);
int rrd_logproc_convert_issue_type(const char *input, char *output, size_t size);
int rrd_logproc_handle_live_logs(const char *source_dir);

#endif // RRD_LOGPROC_H
