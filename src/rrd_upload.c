/*
 * rrd_upload.c - Upload Manager (skeleton)
 */
#include "rrd_upload.h"
#include "rrdCommon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
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

    // 4. Cleanup files - archive and source directory (matching shell script line 143)
    if (rrd_upload_cleanup_files(archive_fullpath, source_dir) != 0) {
        RDK_LOG(RDK_LOG_WARN, LOG_REMDEBUG, "%s: Failed to cleanup files\n", __FUNCTION__);
        return -4;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Cleanup completed\n", __FUNCTION__);
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
    
    // Remove source directory (matching shell script rm -rf $RRD_LOG_PATH)
    if (source_dir) {
        rrd_upload_cleanup_source_dir(source_dir);
    }
    
    return (ret == 0 || !archive_path) ? 0 : -1;
}

// Recursively remove source directory
int rrd_upload_cleanup_source_dir(const char *dir_path) {
    if (!dir_path) return -1;
    
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Removing source directory: %s\n", __FUNCTION__, dir_path);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir_path);
    
    int ret = system(cmd);
    if (ret == 0) {
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Successfully removed source directory: %s\n", 
                __FUNCTION__, dir_path);
        return 0;
    } else {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to remove source directory: %s (ret: %d)\n", 
                __FUNCTION__, dir_path, ret);
        return -1;
    }
}
