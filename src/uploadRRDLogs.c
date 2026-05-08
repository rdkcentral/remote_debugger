/*
 * If not stated otherwise in this file or this component's LICENSE file the
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
/* Use repository logging macro */

// --- Module headers (to be implemented) ---
#include "rrdCommon.h" 
#include "rrd_config.h"    // Configuration Manager
#include "rrd_sysinfo.h"   // System Info Provider
#include "rrd_logproc.h"   // Log Processing Engine
#include "rrd_archive.h"   // Archive Manager
#include "rrd_upload.h"    // Upload Manager
#include "rrd_log.h"       // Logging Subsystem

// --- Main Orchestration Layer ---

int rrd_upload_orchestrate(const char *upload_dir, const char *issue_type)
{
    // Validate input parameters
    if (!upload_dir || !issue_type) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid parameters\n", __FUNCTION__);
        return 1;
    }
    
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Executing binary to upload Debug info of ISSUETYPE=%s\n", __FUNCTION__, issue_type);

    // 2. Initialize logging subsystem
    // Logging is initialized by RDK_LOGGER macros; no explicit init needed
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Logging ready\n", __FUNCTION__);

    // 3. Load configuration via Configuration Manager
    rrd_config_t config;
    if (rrd_config_load(&config) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to load configuration.\n", __FUNCTION__);
        return 3;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Configuration loaded\n", __FUNCTION__);

    // 4. Gather system information
    char mac_addr[32] = {0};
    char timestamp[32] = {0};
    if (rrd_sysinfo_get_mac_address(mac_addr, sizeof(mac_addr)) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to get MAC address.\n", __FUNCTION__);
        return 4;
    }
    if (rrd_sysinfo_get_timestamp(timestamp, sizeof(timestamp)) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to get timestamp.\n", __FUNCTION__);
        return 5;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: MAC: %s, Timestamp: %s\n", __FUNCTION__, mac_addr, timestamp);

    // 5. Validate and prepare log directory
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Checking %s size and contents\n", __FUNCTION__, upload_dir);
    if (rrd_logproc_validate_source(upload_dir) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid or empty upload directory: %s\n", __FUNCTION__, upload_dir);
        return 6;
    }
    if (rrd_logproc_prepare_logs(upload_dir, issue_type) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to prepare logs in %s\n", __FUNCTION__, upload_dir);
        return 7;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Log directory validated and prepared\n", __FUNCTION__);

    // 6. Convert/sanitize issue type
    char issue_type_sanitized[64] = {0};
    if (rrd_logproc_convert_issue_type(issue_type, issue_type_sanitized, sizeof(issue_type_sanitized)) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to sanitize issue type\n", __FUNCTION__);
        return 8;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Issue type sanitized: %s\n", __FUNCTION__, issue_type_sanitized);

    // 6.5. Handle LOGUPLOAD_ENABLE special case (matching shell script lines 128-131)
    if (strcmp(issue_type_sanitized, "LOGUPLOAD_ENABLE") == 0) {
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Check and upload live device logs for the issuetype\n", __FUNCTION__);
        if (rrd_logproc_handle_live_logs(upload_dir) != 0) {
            RDK_LOG(RDK_LOG_WARN, LOG_REMDEBUG, "%s: Failed to handle live logs for LOGUPLOAD_ENABLE\n", __FUNCTION__);
        }
    }

    // 7. Generate archive filename
    char archive_filename[256] = {0};
    if (rrd_archive_generate_filename(mac_addr, issue_type_sanitized, timestamp, archive_filename, sizeof(archive_filename)) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to generate archive filename\n", __FUNCTION__);
        return 9;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Archive filename: %s\n", __FUNCTION__, archive_filename);

    // 8. Create archive in /tmp/rrd/ directory (matching shell script line 127)
    const char *rrd_log_dir = "/tmp/rrd/";
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Creating %s tarfile from Debug Commands output\n", __FUNCTION__, archive_filename);
    if (rrd_archive_create(upload_dir, rrd_log_dir, archive_filename) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to create archive %s\n", __FUNCTION__, archive_filename);
        return 10;
    }

    // 9. Upload archive from /tmp/rrd/ directory
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Invoking uploadSTBLogs binary to upload %s file\n", __FUNCTION__, archive_filename);
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: uploadSTBLogs parameters - server: %s, protocol: %s, http_link: %s, file: %s\n", 
            __FUNCTION__, config.log_server, config.upload_protocol, config.http_upload_link, archive_filename);
    if (rrd_upload_execute(config.log_server, config.upload_protocol, config.http_upload_link, rrd_log_dir, archive_filename, upload_dir) != 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: RRD %s Debug Information Report upload Failed!!!\n", __FUNCTION__, issue_type_sanitized);
        // Cleanup on failure (matching shell script lines 139-140)
        char archive_fullpath[512];
        snprintf(archive_fullpath, sizeof(archive_fullpath), "%s%s", rrd_log_dir, archive_filename);
        rrd_archive_cleanup(archive_fullpath);
        rrd_upload_cleanup_source_dir(upload_dir);
        return 11;
    }
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: RRD %s Debug Information Report upload Success\n", __FUNCTION__, issue_type_sanitized);

    // 10. Cleanup archive and source directory (matching shell script line 143)
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Removing uploaded report %s\n", __FUNCTION__, archive_filename);
    char archive_fullpath[512];
    snprintf(archive_fullpath, sizeof(archive_fullpath), "%s%s", rrd_log_dir, archive_filename);
    rrd_archive_cleanup(archive_fullpath);
    rrd_upload_cleanup_source_dir(upload_dir);

    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Exit\n", __FUNCTION__);
    return 0;
}


