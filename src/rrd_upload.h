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


#ifndef RRD_UPLOAD_H
#define RRD_UPLOAD_H

#include <stdbool.h>
#include "rrdCommon.h"
#ifndef GTEST_ENABLE
#ifdef IARMBUS_SUPPORT
#include <uploadstblogs.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

int rrd_upload_execute(const char *log_server, const char *protocol, const char *http_link, const char *working_dir, const char *archive_filename, const char *source_dir);
int rrd_upload_check_lock(bool *is_locked);
int rrd_upload_wait_for_lock(int max_attempts, int wait_seconds);
int rrd_upload_invoke_logupload_api(const char *log_server, const char *protocol, const char *http_link, const char *archive_filename);
int rrd_upload_orchestrate(const char *upload_dir, const char *issue_type);
int rrd_upload_cleanup_files(const char *archive_path, const char *source_dir);
int rrd_upload_cleanup_source_dir(const char *dir_path);
void rrd_upload_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // RRD_UPLOAD_H
