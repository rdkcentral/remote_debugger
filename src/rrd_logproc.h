/*
 * rrd_logproc.h - Log Processing Engine (skeleton)
 */
#ifndef RRD_LOGPROC_H
#define RRD_LOGPROC_H

#include <stddef.h>

int rrd_logproc_validate_source(const char *source_dir);
int rrd_logproc_prepare_logs(const char *source_dir, const char *issue_type);
int rrd_logproc_convert_issue_type(const char *input, char *output, size_t size);
int rrd_logproc_handle_live_logs(const char *source_dir);

#endif // RRD_LOGPROC_H
