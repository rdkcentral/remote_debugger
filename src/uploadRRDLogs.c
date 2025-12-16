/*
 * uploadRRDLogs.c - Skeleton for C migration of uploadRRDLogs.sh
 *
 * This file is auto-generated following HLD and implementation instructions.
 *
 * Modules: Main Orchestration, Config Manager, System Info, Log Processing, Archive, Upload, Logging
 * See: .github/docs/uploadRRDLogs_HLD.md
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <rdk_logger.h>

/* Use repository logging macro */

// --- Module headers (to be implemented) ---
#include "rrd_config.h"    // Configuration Manager
#include "rrd_sysinfo.h"   // System Info Provider
#include "rrd_logproc.h"   // Log Processing Engine
#include "rrd_archive.h"   // Archive Manager
#include "rrd_upload.h"    // Upload Manager
#include "rrd_log.h"       // Logging Subsystem

// --- Main Orchestration Layer ---


int main(int argc, char *argv[])
{
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Entry\n", __FUNCTION__);
    // 1. Parse and validate command-line arguments (UPLOADDIR, ISSUETYPE)
    if (argc != 3) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Usage: %s UPLOADDIR ISSUETYPE\n", __FUNCTION__, argv[0]);
        fprintf(stderr, "Usage: %s UPLOADDIR ISSUETYPE\n", argv[0]);
        return 1;
    }
    const char *upload_dir = argv[1];
    const char *issue_type = argv[2];

    // 2. Initialize logging subsystem
    // Logging is initialized by RDK_LOGGER macros; no explicit init needed
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Logging ready\n", __FUNCTION__);

    // 3. Load configuration via Configuration Manager
    rrd_config_t config;
    if (rrd_config_load(&config) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to load configuration.\n", __FUNCTION__);
        fprintf(stderr, "Failed to load configuration.\n");
        return 3;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Configuration loaded\n", __FUNCTION__);

    // 4. Gather system information
    char mac_addr[32] = {0};
    char timestamp[32] = {0};
    if (rrd_sysinfo_get_mac_address(mac_addr, sizeof(mac_addr)) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to get MAC address.\n", __FUNCTION__);
        fprintf(stderr, "Failed to get MAC address.\n");
        return 4;
    }
    if (rrd_sysinfo_get_timestamp(timestamp, sizeof(timestamp)) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to get timestamp.\n", __FUNCTION__);
        fprintf(stderr, "Failed to get timestamp.\n");
        return 5;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: MAC: %s, Timestamp: %s\n", __FUNCTION__, mac_addr, timestamp);

    // 5. Validate and prepare log directory
    if (rrd_logproc_validate_source(upload_dir) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid or empty upload directory: %s\n", __FUNCTION__, upload_dir);
        fprintf(stderr, "Invalid or empty upload directory: %s\n", upload_dir);
        return 6;
    }
    if (rrd_logproc_prepare_logs(upload_dir, issue_type) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to prepare logs in %s\n", __FUNCTION__, upload_dir);
        fprintf(stderr, "Failed to prepare logs in %s\n", upload_dir);
        return 7;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Log directory validated and prepared\n", __FUNCTION__);

    // 6. Convert/sanitize issue type
    char issue_type_sanitized[64] = {0};
    if (rrd_logproc_convert_issue_type(issue_type, issue_type_sanitized, sizeof(issue_type_sanitized)) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to sanitize issue type\n", __FUNCTION__);
        fprintf(stderr, "Failed to sanitize issue type\n");
        return 8;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Issue type sanitized: %s\n", __FUNCTION__, issue_type_sanitized);

    // 7. Generate archive filename
    char archive_filename[256] = {0};
    if (rrd_archive_generate_filename(mac_addr, issue_type_sanitized, timestamp, archive_filename, sizeof(archive_filename)) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to generate archive filename\n", __FUNCTION__);
        fprintf(stderr, "Failed to generate archive filename\n");
        return 9;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Archive filename: %s\n", __FUNCTION__, archive_filename);

    // 8. Create archive
    if (rrd_archive_create(upload_dir, NULL, archive_filename) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to create archive %s\n", __FUNCTION__, archive_filename);
        fprintf(stderr, "Failed to create archive %s\n", archive_filename);
        return 10;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Archive created: %s\n", __FUNCTION__, archive_filename);

    // 9. Upload archive
    if (rrd_upload_execute(config.log_server, config.upload_protocol, config.http_upload_link, upload_dir, archive_filename) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to upload archive\n", __FUNCTION__);
        fprintf(stderr, "Failed to upload archive\n");
        rrd_archive_cleanup(archive_filename);
        return 11;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Archive uploaded successfully\n", __FUNCTION__);

    // 10. Cleanup
    rrd_archive_cleanup(archive_filename);
    rrd_upload_cleanup();
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Cleanup complete\n", __FUNCTION__);

    printf("Upload completed successfully.\n");
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Exit\n", __FUNCTION__);
    return 0;
}


int rrd_upload_orchestrate(const char *upload_dir, const char *issue_type)
{
    // Deprecated: logic now in main()
    (void)upload_dir; (void)issue_type;
    return 0;
}


void rrd_upload_cleanup(void)
{
    // Placeholder for any additional cleanup (temp files, etc.)
}

// --- End of Skeleton ---
