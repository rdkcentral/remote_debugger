/*
 * rrd_logproc.c - Log Processing Engine (skeleton)
 */
#include "rrd_logproc.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>

// Validate source directory: must exist, be a directory, and not empty
int rrd_logproc_validate_source(const char *source_dir) {
    if (!source_dir) return -1;
    struct stat st;
    if (stat(source_dir, &st) != 0) return -2;
    if (!S_ISDIR(st.st_mode)) return -3;
    DIR *d = opendir(source_dir);
    if (!d) return -4;
    struct dirent *ent;
    int found = 0;
    while ((ent = readdir(d))) {
        if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
            found = 1; break;
        }
    }
    closedir(d);
    return found ? 0 : -5;
}

// Prepare logs for archiving: could filter, copy, or compress logs as needed
int rrd_logproc_prepare_logs(const char *source_dir, const char *issue_type) {
    // For now, just validate source and return
    int valid = rrd_logproc_validate_source(source_dir);
    if (valid != 0) return valid;
    // In a real system, could filter logs by issue_type, copy to temp dir, etc.
    (void)issue_type;
    return 0;
}

// Convert issue type to uppercase and sanitize (alnum/underscore only)
int rrd_logproc_convert_issue_type(const char *input, char *output, size_t size) {
    if (!input || !output || size == 0) return -1;
    size_t j = 0;
    for (size_t i = 0; input[i] && j < size-1; ++i) {
        char c = input[i];
        if (isalnum((unsigned char)c)) output[j++] = toupper((unsigned char)c);
        else if (c == '_' || c == '-') output[j++] = '_';
        // skip other chars
    }
    output[j] = 0;
    return 0;
}

// Handle live logs for LOGUPLOAD_ENABLE: could tail/follow logs, or copy latest
int rrd_logproc_handle_live_logs(const char *source_dir) {
    // For now, just validate source
    int valid = rrd_logproc_validate_source(source_dir);
    if (valid != 0) return valid;
    // In a real system, could tail logs, copy new logs, etc.
    return 0;
}
