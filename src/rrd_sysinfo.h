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


#ifndef RRD_SYSINFO_H
#define RRD_SYSINFO_H

#include <stdbool.h>
#include <stddef.h>
#include "rrdCommon.h"
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#ifndef GTEST_ENABLE
#include <common_device_api.h>
#endif

/* Get the device MAC address.
 * @param mac_addr Buffer to store MAC address (min 18 bytes)
 * @param size Size of buffer
 * @return 0 on success, -1 on error
 */
int rrd_sysinfo_get_mac_address(char *mac_addr, size_t size);

/* Get formatted timestamp string.
 * @param timestamp Buffer to store timestamp
 * @param size Size of buffer
 * @return 0 on success, -1 on error
 */
int rrd_sysinfo_get_timestamp(char *timestamp, size_t size);

/* Check if a file exists. */
bool rrd_sysinfo_file_exists(const char *filepath);

/* Check if a directory exists. */
bool rrd_sysinfo_dir_exists(const char *dirpath);

/* Check if a directory is empty. */
bool rrd_sysinfo_dir_is_empty(const char *dirpath);

/* Get total size of files in a directory (recursive). */
int rrd_sysinfo_get_dir_size(const char *dirpath, size_t *size);

#endif /* RRD_SYSINFO_H */
