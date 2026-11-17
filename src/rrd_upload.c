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

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "rrd_upload.h"
#include "rrdCommon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ftw.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>




int rrd_upload_execute(const char *log_server, const char *protocol, const char *http_link, const char *working_dir, const char *archive_filename, const char *source_dir) {
    // Validate required parameters
    if (!log_server || strlen(log_server) == 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid or empty log_server\n", __FUNCTION__);
        return -1;
    }
    if (!protocol) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid upload protocol\n", __FUNCTION__);
        return -1;
    }
    if (!http_link) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid HTTP upload link\n", __FUNCTION__);
        return -1;
    }
    if (!working_dir) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid working directory\n", __FUNCTION__);
        return -1;
    }
    if (!archive_filename) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid archive filename\n", __FUNCTION__);
        return -1;
    }
    
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Starting upload - server: %s, protocol: %s, file: %s\n", 
            __FUNCTION__, log_server, protocol, archive_filename);
    
    // 1. Check for upload lock (matching shell script line 67)
    bool locked = false;
    if (rrd_upload_check_lock(&locked) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to check upload lock\n", __FUNCTION__);
        return -1;
    }
    if (locked) {
        RDK_LOG(RDK_LOG_WARN, LOG_REMDEBUG, "%s: Upload lock detected, waiting...\n", __FUNCTION__);
        if (rrd_upload_wait_for_lock(10, 60) != 0) {
            RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Upload lock timeout\n", __FUNCTION__);
            return -2;
        }
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Upload lock cleared\n", __FUNCTION__);
    }

    // 2. Prepare full path to archive file
    char archive_fullpath[512];
    snprintf(archive_fullpath, sizeof(archive_fullpath), "%s%s", working_dir, archive_filename);
    
    // 3. Prepare parameters for uploadstblogs_run
    UploadSTBLogsParams params = {
        .flag = 1,
        .dcm_flag = 0, // Not a DCM-triggered upload
        .upload_on_reboot = false,
        .upload_protocol = protocol,
        .upload_http_link = http_link,
        .trigger_type = TRIGGER_ONDEMAND,
        .rrd_flag = true,
        .rrd_file = archive_fullpath
    };

    int result = uploadstblogs_run(&params);
    if (result != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Log upload failed with error code: %d\n", __FUNCTION__, result);
        fprintf(stderr, "Log upload failed: %d\n", result);
        return -3;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Upload completed successfully\n", __FUNCTION__);
    
    return 0;
}

// Check for concurrent upload lock file (matching uploadstblogs binary)
int rrd_upload_check_lock(bool *is_locked) {
    if (!is_locked) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid is_locked pointer\n", __FUNCTION__);
        return -1;
    }
    
    // Try to acquire a non-blocking lock on the same file uploadstblogs uses
    int fd = open("/tmp/.log-upload.lock", O_RDONLY | O_CREAT, 0644);
    if (fd == -1) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to open lock file (errno: %d)\n", __FUNCTION__, errno);
        return -1;
    }
    
    // Try to acquire a non-blocking shared lock (LOCK_SH | LOCK_NB)
    // If uploadstblogs has an exclusive lock (LOCK_EX), this will fail with EWOULDBLOCK
    int lock_ret = flock(fd, LOCK_SH | LOCK_NB);
    if (lock_ret == 0) {
        // Successfully acquired shared lock - no exclusive lock held
        *is_locked = false;
        flock(fd, LOCK_UN);  // Release our shared lock
        close(fd);
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Lock status: free\n", __FUNCTION__);
    } else if (errno == EWOULDBLOCK || errno == EAGAIN) {
        // Exclusive lock is held by another process
        *is_locked = true;
        close(fd);
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Lock status: locked\n", __FUNCTION__);
    } else {
        // Some other error
        close(fd);
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: flock failed (errno: %d)\n", __FUNCTION__, errno);
        return -1;
    }
    
    return 0;
}

// Wait for lock file to clear (matching uploadstblogs binary lock)
int rrd_upload_wait_for_lock(int max_attempts, int wait_seconds) {
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Waiting for upload lock to clear (max attempts: %d, wait: %ds)\n", 
            __FUNCTION__, max_attempts, wait_seconds);
    
    for (int i = 0; i < max_attempts; ++i) {
        bool locked = false;
        if (rrd_upload_check_lock(&locked) == 0 && !locked) {
            RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Lock cleared after %d attempt(s)\n", __FUNCTION__, i + 1);
            return 0; // lock gone
        }
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Lock still present, attempt %d/%d, sleeping %ds...\n", 
                __FUNCTION__, i + 1, max_attempts, wait_seconds);
        sleep(wait_seconds);
    }
    RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Lock timeout after %d attempts\n", __FUNCTION__, max_attempts);
    return -1; // still locked
}


// All log upload is now handled via dcm-agent's uploadstblogs_run API.

// Cleanup files after upload (matching shell script line 139 and 143)
int rrd_upload_cleanup_files(const char *archive_path, const char *source_dir) {
    int ret = 0;
    int source_ret = 0;
    
    // Remove archive file
    if (archive_path) {
        ret = remove(archive_path);
        if (ret == 0) {
            RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Removed archive: %s\n", __FUNCTION__, archive_path);
        } else {
            RDK_LOG(RDK_LOG_WARN, LOG_REMDEBUG, "%s: Failed to remove archive: %s (errno: %d)\n", 
                    __FUNCTION__, archive_path, errno);
        }
    }
    
    // Remove source directory with the filesystem cleanup routine.
    if (source_dir) {
        source_ret = rrd_upload_cleanup_source_dir(source_dir);
    }
    
    return (ret == 0 || !archive_path) && source_ret == 0 ? 0 : -1;
}

static int rrd_upload_remove_entry(const char *path, const struct stat *st,
                                   int type, struct FTW *info)
{
    (void)st;
    (void)info;
    if (type == FTW_NS || type == FTW_DNR) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Cannot traverse %s (errno: %d)\n",
                __FUNCTION__, path, errno);
        return -1;
    }
    if (remove(path) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to remove %s (errno: %d)\n",
                __FUNCTION__, path, errno);
        return -1;
    }
    return 0;
}

static bool rrd_upload_is_report_source_dir(const char *dir_path)
{
    static const char report_root[] = "/tmp/rrd/";
    const size_t report_root_len = sizeof(report_root) - 1;

    if (strncmp(dir_path, report_root, report_root_len) != 0) {
        return false;
    }

    const char *name = dir_path + report_root_len;
    return name[0] != '\0' && strchr(name, '/') == NULL &&
           strcmp(name, ".") != 0 && strcmp(name, "..") != 0;
}

int rrd_upload_cleanup_source_dir(const char *dir_path)
{
    if (!dir_path || dir_path[0] == '\0') {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid source directory path\n", __FUNCTION__);
        return -1;
    }

    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Removing source directory: %s\n", __FUNCTION__, dir_path);

    if (!rrd_upload_is_report_source_dir(dir_path)) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Refusing to remove path outside report directory: %s\n",
                __FUNCTION__, dir_path);
        return -1;
    }

    int fd = open(dir_path, O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
    if (fd < 0) {
       if (errno == ENOENT) {
           return 0;
       }
       return -1;
     }

    struct stat st;
    if (fstat(fd, &st) != 0) {
            close(fd);
            RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to inspect source directory %s (errno: %d)\n",
                __FUNCTION__, dir_path, errno);
            return -1;
        }

    if (!S_ISDIR(st.st_mode)) {
        close(fd);
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Source path is not a directory: %s\n",
                __FUNCTION__, dir_path);
        return -1;
    }

    close(fd);

    if (nftw(dir_path, rrd_upload_remove_entry, 8, FTW_DEPTH | FTW_PHYS) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to remove source directory %s (errno: %d)\n",
                __FUNCTION__, dir_path, errno);
        return -1;
    }

    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Successfully removed source directory: %s\n",
            __FUNCTION__, dir_path);
    return 0;
}
