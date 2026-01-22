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

#include "rrd_sysinfo.h"


/* Use repository logging macro */



int rrd_sysinfo_get_mac_address(char *mac_addr, size_t size) {
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Entry\n", __FUNCTION__);
    if (!mac_addr || size < 13) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid MAC buffer or size (need at least 13 bytes)\n", __FUNCTION__);
        return -1;
    }
    
    // Get MAC address with colons (e.g., "D4:52:EE:58:F6:AE")
    char mac_with_colons[18] = {0};
    size_t copied = GetEstbMac(mac_with_colons, sizeof(mac_with_colons));
    if (copied == 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to get MAC address\n", __FUNCTION__);
        return -1;
    }
    
    // Strip colons 
    memset(mac_addr, 0, size);
    size_t j = 0;
    for (size_t i = 0; mac_with_colons[i] != '\0' && j < size - 1; i++) {
        if (mac_with_colons[i] != ':') {
            mac_addr[j++] = mac_with_colons[i];
        }
    }
    mac_addr[j] = '\0';
    
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: MAC address obtained: %s\n", __FUNCTION__, mac_addr);
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Exit\n", __FUNCTION__);
    return 0;
}



int rrd_sysinfo_get_timestamp(char *timestamp, size_t size) {
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Entry\n", __FUNCTION__);
    if (!timestamp || size < 20) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid timestamp buffer or size\n", __FUNCTION__);
        return -1;
    }
    memset(timestamp, 0, size);
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    if (!tm_info) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to get local time\n", __FUNCTION__);
        return -1;
    }
    char ampm[3] = "AM";
    int hour = tm_info->tm_hour;
    if (hour >= 12) {
        strcpy(ampm, "PM");
        if (hour > 12) hour -= 12;
    } else if (hour == 0) {
        hour = 12;
    }
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d-%02d-%02d-%02d%s",
        tm_info->tm_year + 1900,
        tm_info->tm_mon + 1,
        tm_info->tm_mday,
        hour,
        tm_info->tm_min,
        tm_info->tm_sec,
        ampm);
    strncpy(timestamp, buf, size - 1);
    timestamp[size - 1] = '\0';
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Timestamp generated: %s\n", __FUNCTION__, timestamp);
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Exit\n", __FUNCTION__);
    return 0;
}

bool rrd_sysinfo_file_exists(const char *filepath) {
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Entry\n", __FUNCTION__);
    if (!filepath) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid filepath\n", __FUNCTION__);
        return false;
    }
    bool exists = access(filepath, F_OK) == 0;
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: File %s exists: %d\n", __FUNCTION__, filepath, exists);
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Exit\n", __FUNCTION__);
    return exists;
}

bool rrd_sysinfo_dir_exists(const char *dirpath) {
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Entry\n", __FUNCTION__);
    if (!dirpath) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid dirpath\n", __FUNCTION__);
        return false;
    }
    struct stat st;
    bool exists = (stat(dirpath, &st) == 0 && S_ISDIR(st.st_mode));
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Directory %s exists: %d\n", __FUNCTION__, dirpath, exists);
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Exit\n", __FUNCTION__);
    return exists;
}

bool rrd_sysinfo_dir_is_empty(const char *dirpath) {
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Entry\n", __FUNCTION__);
    if (!dirpath) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid dirpath\n", __FUNCTION__);
        return false;
    }
    DIR *dir = opendir(dirpath);
    if (!dir) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to open directory %s\n", __FUNCTION__, dirpath);
        return false;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            closedir(dir);
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Directory %s is not empty\n", __FUNCTION__, dirpath);
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Exit\n", __FUNCTION__);
            return false; // Found a file/dir
        }
    }
    closedir(dir);
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Directory %s is empty\n", __FUNCTION__, dirpath);
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Exit\n", __FUNCTION__);
    return true; // No files/dirs found
}

int rrd_sysinfo_get_dir_size(const char *dirpath, size_t *size) {
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Entry\n", __FUNCTION__);
    if (!dirpath || !size) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid dirpath or size pointer\n", __FUNCTION__);
        return -1;
    }
    *size = 0;
    DIR *dir = opendir(dirpath);
    if (!dir) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to open directory %s\n", __FUNCTION__, dirpath);
        return -1;
    }
    struct dirent *entry;
    char filepath[1024] = {0};
    struct stat st;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, entry->d_name);
        if (stat(filepath, &st) == 0) {
            if (S_ISREG(st.st_mode)) {
                *size += st.st_size;
            } else if (S_ISDIR(st.st_mode)) {
                size_t subdir_size = 0;
                if (rrd_sysinfo_get_dir_size(filepath, &subdir_size) == 0) {
                    *size += subdir_size;
                }
            }
        }
    }
    closedir(dir);
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Directory %s total size: %zu bytes\n", __FUNCTION__, dirpath, *size);
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Exit\n", __FUNCTION__);
    return 0;
}
