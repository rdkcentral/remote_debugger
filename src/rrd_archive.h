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

#ifndef RRD_ARCHIVE_H
#define RRD_ARCHIVE_H

#include <stddef.h>

int rrd_archive_create(const char *source_dir, const char *working_dir, const char *archive_filename);
/* suffix is optional (NULL or ""); when non-empty it is appended AFTER the
 * timestamp field: {MAC}_{ISSUE_TYPE}_{TIMESTAMP}_{SUFFIX}_RRD_DEBUG_LOGS.tgz */
int rrd_archive_generate_filename(const char *mac, const char *issue_type, const char *timestamp, const char *suffix, char *filename, size_t size);
int rrd_archive_cleanup(const char *archive_path);
int rrd_archive_check_cpu_usage(float *cpu_usage);
int rrd_archive_adjust_priority(float cpu_usage);

#endif // RRD_ARCHIVE_H
