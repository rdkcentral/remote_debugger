/*
 * rrd_upload.h - Upload Manager (skeleton)
 */
#ifndef RRD_UPLOAD_H
#define RRD_UPLOAD_H

#include <stdbool.h>
#include "rrdCommon.h"
#include "uploadstblogs.h" 


int rrd_upload_execute(const char *log_server, const char *protocol, const char *http_link, const char *working_dir, const char *archive_filename);
int rrd_upload_check_lock(bool *is_locked);
int rrd_upload_wait_for_lock(int max_attempts, int wait_seconds);
int rrd_upload_invoke_logupload_api(const char *log_server, const char *protocol, const char *http_link, const char *archive_filename);
int rrd_upload_cleanup_files(const char *archive_path, const char *source_dir);
void rrd_upload_cleanup(void);

#endif // RRD_UPLOAD_H
