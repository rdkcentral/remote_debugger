/*
 * rrd_logproc.c - Log Processing Engine (skeleton)
 */
#include "rrd_logproc.h"
#include "rrdCommon.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>

// Validate source directory: must exist, be a directory, and not empty
int rrd_logproc_validate_source(const char *source_dir) {
    if (!source_dir) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: NULL source directory\n", __FUNCTION__);
        return -1;
    }
    
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Validating source: %s\n", __FUNCTION__, source_dir);
    
    struct stat st;
    if (stat(source_dir, &st) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Directory does not exist: %s (errno: %d)\n", 
                __FUNCTION__, source_dir, errno);
        return -2;
    }
    
    if (!S_ISDIR(st.st_mode)) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Path is not a directory: %s\n", __FUNCTION__, source_dir);
        return -3;
    }
    
    DIR *d = opendir(source_dir);
    if (!d) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Cannot open directory: %s (errno: %d)\n", 
                __FUNCTION__, source_dir, errno);
        return -4;
    }
    
    struct dirent *ent;
    int found = 0;
    while ((ent = readdir(d))) {
        if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
            found = 1; break;
        }
    }
    closedir(d);
    
    if (!found) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Directory is empty: %s\n", __FUNCTION__, source_dir);
        return -5;
    }
    
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Source directory validated successfully: %s\n", 
            __FUNCTION__, source_dir);
    return 0;
}

// Prepare logs for archiving: could filter, copy, or compress logs as needed
int rrd_logproc_prepare_logs(const char *source_dir, const char *issue_type) {
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Entry - source: %s, issue_type: %s\n", 
            __FUNCTION__, source_dir ? source_dir : "NULL", issue_type ? issue_type : "NULL");
    
    // Validate parameters
    if (!issue_type) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: NULL issue_type\n", __FUNCTION__);
        return -1;
    }
    
    // Validate source directory
    int valid = rrd_logproc_validate_source(source_dir);
    if (valid != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Source validation failed with code: %d\n", 
                __FUNCTION__, valid);
        return valid;
    }
    
    // In a real system, could filter logs by issue_type, copy to temp dir, etc.
    (void)issue_type;
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Logs prepared successfully\n", __FUNCTION__);
    return 0;
}

// Convert issue type to uppercase and sanitize (alnum/underscore only)
int rrd_logproc_convert_issue_type(const char *input, char *output, size_t size) {
    if (!input) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: NULL input\n", __FUNCTION__);
        return -1;
    }
    if (!output) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: NULL output buffer\n", __FUNCTION__);
        return -1;
    }
    if (size == 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Zero buffer size\n", __FUNCTION__);
        return -1;
    }
    
    size_t len = strlen(input);
    if (len >= size) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Buffer too small (need: %zu, have: %zu)\n", 
                __FUNCTION__, len + 1, size);
        return -1;  // Buffer too small
    }
    
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Converting issue type: %s\n", __FUNCTION__, input);
    
    size_t j = 0;
    for (size_t i = 0; input[i] && j < size-1; ++i) {
        char c = input[i];
        if (isalnum((unsigned char)c)) output[j++] = toupper((unsigned char)c);
        else if (c == '_' || c == '-' || c == '.') output[j++] = '_';
        // skip other chars
    }
    output[j] = 0;
    
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Converted '%s' to '%s'\n", __FUNCTION__, input, output);
    return 0;
}

// Handle live logs for LOGUPLOAD_ENABLE: could tail/follow logs, or copy latest
int rrd_logproc_handle_live_logs(const char *source_dir) {
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Entry - source: %s\n", 
            __FUNCTION__, source_dir ? source_dir : "NULL");
    
    // For now, just validate source
    int valid = rrd_logproc_validate_source(source_dir);
    if (valid != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Source validation failed with code: %d\n", 
                __FUNCTION__, valid);
        return valid;
    }
    
    // In a real system, could tail logs, copy new logs, etc.
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Live logs handled successfully\n", __FUNCTION__);
    return 0;
}
