/*
 * rrd_archive.c - Archive Manager (skeleton)
 */
#include "rrd_archive.h"

// Create gzip-compressed tar archive using system tar/gzip (fallback if no libarchive)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

int rrd_archive_create(const char *source_dir, const char *working_dir, const char *archive_filename) {
    if (!source_dir || !archive_filename) return -1;
    char cmd[1024];
    // Compose tar command: tar -czf <archive_filename> -C <source_dir> .
    if (working_dir && strlen(working_dir) > 0) {
        snprintf(cmd, sizeof(cmd), "cd '%s' && tar -czf '%s' -C '%s' .", working_dir, archive_filename, source_dir);
    } else {
        snprintf(cmd, sizeof(cmd), "tar -czf '%s' -C '%s' .", archive_filename, source_dir);
    }
    int ret = system(cmd);
    if (ret != 0) return -2;
    struct stat st;
    if (stat(archive_filename, &st) != 0 || st.st_size == 0) return -3;
    return 0;
}

// Generate archive filename: <mac>_<issue_type>_<timestamp>.tar.gz
int rrd_archive_generate_filename(const char *mac, const char *issue_type, const char *timestamp, char *filename, size_t size) {
    if (!mac || !issue_type || !timestamp || !filename || size < 16) return -1;
    snprintf(filename, size, "%s_%s_%s.tar.gz", mac, issue_type, timestamp);
    return 0;
}

int rrd_archive_cleanup(const char *archive_path) {
    if (!archive_path) return -1;
    int ret = remove(archive_path);
    return (ret == 0) ? 0 : -2;
}

// Check system CPU usage (Linux: parse /proc/stat)
int rrd_archive_check_cpu_usage(float *cpu_usage) {
    if (!cpu_usage) return -1;
    FILE *f = fopen("/proc/stat", "r");
    if (!f) return -2;
    char buf[256];
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    if (!fgets(buf, sizeof(buf), f)) { fclose(f); return -3; }
    fclose(f);
    int n = sscanf(buf, "cpu %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
    if (n < 4) return -4;
    unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal;
    if (total == 0) return -5;
    *cpu_usage = 100.0f * (float)(total - idle) / (float)total;
    return 0;
}

// Adjust process priority based on CPU usage (lower priority if high usage)
#include <sys/resource.h>
int rrd_archive_adjust_priority(float cpu_usage) {
    int prio = 0;
    if (cpu_usage > 80.0f) prio = 19; // lowest
    else if (cpu_usage > 50.0f) prio = 10;
    else prio = 0; // normal
    int ret = setpriority(PRIO_PROCESS, 0, prio);
    return (ret == 0) ? 0 : -1;
}
