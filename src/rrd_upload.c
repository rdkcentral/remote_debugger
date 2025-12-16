/*
 * rrd_upload.c - Upload Manager (skeleton)
 */
#include "rrd_upload.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "uploadstblogs.h" // For UploadSTBLogsParams and uploadstblogs_run


int rrd_upload_execute(const char *log_server, const char *protocol, const char *http_link, const char *working_dir, const char *archive_filename) {
    // 1. Check for upload lock
    bool locked = false;
    if (rrd_upload_check_lock(&locked) != 0) return -1;
    if (locked) {
        if (rrd_upload_wait_for_lock(10, 2) != 0) return -2;
    }

    // 2. Prepare parameters for uploadstblogs_run
    UploadSTBLogsParams params = {
        .flag = 1,
        .dcm_flag = 0, // Not a DCM-triggered upload
        .upload_on_reboot = false,
        .upload_protocol = protocol ? protocol : "HTTP",
        .upload_http_link = http_link ? http_link : "",
        .trigger_type = TRIGGER_ONDEMAND,
        .rrd_flag = true,
        .rrd_file = archive_filename
    };

    int result = uploadstblogs_run(&params);
    if (result != 0) {
        fprintf(stderr, "Log upload failed: %d\n", result);
        return -3;
    }

    // 3. Cleanup files
    if (rrd_upload_cleanup_files(archive_filename, working_dir) != 0) return -4;
    return 0;
}

// Check for concurrent upload lock file
int rrd_upload_check_lock(bool *is_locked) {
    if (!is_locked) return -1;
    struct stat st;
    int ret = stat("/tmp/rrd_upload.lock", &st);
    *is_locked = (ret == 0);
    return 0;
}

// Wait for lock file to clear
int rrd_upload_wait_for_lock(int max_attempts, int wait_seconds) {
    for (int i = 0; i < max_attempts; ++i) {
        struct stat st;
        if (stat("/tmp/rrd_upload.lock", &st) != 0) return 0; // lock gone
        sleep(wait_seconds);
    }
    return -1; // still locked
}


// All log upload is now handled via dcm-agent's uploadstblogs_run API.

// Cleanup files after upload
int rrd_upload_cleanup_files(const char *archive_path, const char *source_dir) {
    int ret = 0;
    if (archive_path) ret = remove(archive_path);
    // Optionally, could clean up working dir or temp files in source_dir
    (void)source_dir;
    return (ret == 0 || !archive_path) ? 0 : -1;
}
