/*
 * rrd_archive.h - Archive Manager (skeleton)
 */
#ifndef RRD_ARCHIVE_H
#define RRD_ARCHIVE_H

#include <stddef.h>

int rrd_archive_create(const char *source_dir, const char *working_dir, const char *archive_filename);
int rrd_archive_generate_filename(const char *mac, const char *issue_type, const char *timestamp, char *filename, size_t size);
int rrd_archive_cleanup(const char *archive_path);
int rrd_archive_check_cpu_usage(float *cpu_usage);
int rrd_archive_adjust_priority(float cpu_usage);

#endif // RRD_ARCHIVE_H
