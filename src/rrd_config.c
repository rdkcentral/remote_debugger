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

#include "rrd_config.h"
#include "rrdCommon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

// Helper: trim whitespace
static void trim(char *str) {
    if (!str) return;
    char *start = str;
    char *end;
    
    // Trim leading
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') start++;
    
    // Trim trailing
    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) *end-- = 0;
    
    // Move trimmed string to beginning if needed
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

// Helper: execute command and capture output
static int execute_command(const char *cmd, char *output, size_t output_size) {
    if (!cmd || !output || output_size == 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid parameters\n", __FUNCTION__);
        return -1;
    }
    
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Executing command: %s\n", __FUNCTION__, cmd);
    
    output[0] = '\0';
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Failed to execute command: %s\n", __FUNCTION__, cmd);
        return -1;
    }
    
    if (fgets(output, output_size, fp) != NULL) {
        // Remove trailing newline
        size_t len = strlen(output);
        if (len > 0 && output[len-1] == '\n') {
            output[len-1] = '\0';
        }
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Command output: %s\n", __FUNCTION__, output);
    } else {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: No output from command\n", __FUNCTION__);
    }
    
    int status = pclose(fp);
    if (status != 0) {
        RDK_LOG(RDK_LOG_WARN, LOG_REMDEBUG, "%s: Command exited with status: %d\n", __FUNCTION__, status);
    }
    
    return (status == 0 && strlen(output) > 0) ? 0 : -1;
}

// Helper: check if file exists
static bool file_exists(const char *filepath) {
    if (!filepath) return false;
    struct stat st;
    return (stat(filepath, &st) == 0);
}

int rrd_config_load(rrd_config_t *config) {
    if (!config) return -1;
    
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Loading configuration...\n", __FUNCTION__);
    
    memset(config, 0, sizeof(*config));
    config->use_rfc_config = false;
    
    // Set default protocol
    strncpy(config->upload_protocol, "HTTP", sizeof(config->upload_protocol)-1);
    
    // 1. Parse /etc/include.properties and /etc/device.properties
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Parsing /etc/include.properties\n", __FUNCTION__);
    int prop_ok = rrd_config_parse_properties("/etc/include.properties", config);
    if (prop_ok == 0) {
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Loaded /etc/include.properties\n", __FUNCTION__);
    }
    
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Parsing /etc/device.properties\n", __FUNCTION__);
    rrd_config_parse_properties("/etc/device.properties", config);
    
    // Check BUILD_TYPE for prod override logic (matching shell script lines 81-83)
    bool is_prod_with_override = (strcmp(config->build_type, "prod") != 0 && file_exists("/opt/dcm.properties"));
    if (is_prod_with_override) {
        RDK_LOG(RDK_LOG_WARN, LOG_REMDEBUG, 
                "%s: Configurable service end-points will not be used for %s Builds due to overriden /opt/dcm.properties!!!\n",
                __FUNCTION__, config->build_type);
    } else {
        // 2. Try RFC query via tr181 (matching shell script lines 84-100)
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Querying RFC configuration\n", __FUNCTION__);
        int rfc_ok = rrd_config_query_rfc(config);
        if (rfc_ok == 0) {
            config->use_rfc_config = true;
            RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: RFC configuration loaded\n", __FUNCTION__);
        }
        
        // 3. Parse DCM settings from /tmp/DCMSettings.conf (matching shell script lines 101-113)
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Parsing /tmp/DCMSettings.conf\n", __FUNCTION__);
        int dcm_ok = rrd_config_parse_dcm_settings("/tmp/DCMSettings.conf", config);
        if (dcm_ok == 0) {
            RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Loaded /tmp/DCMSettings.conf\n", __FUNCTION__);
        }
    }
    
    // 4. Final fallback: if LOG_SERVER or HTTP_UPLOAD_LINK is still empty, try dcm.properties
    config->log_server[sizeof(config->log_server)-1] = '\0';
    config->http_upload_link[sizeof(config->http_upload_link)-1] = '\0';
    if (strlen(config->log_server) == 0 || strlen(config->http_upload_link) == 0) {
        RDK_LOG(RDK_LOG_WARN, LOG_REMDEBUG, 
                "%s: DCM params read using RFC/tr181 is empty, trying dcm.properties fallback\n", 
                __FUNCTION__);
        
        const char *dcm_file = NULL;
        if (strcmp(config->build_type, "prod") != 0 && file_exists("/opt/dcm.properties")) {
            dcm_file = "/opt/dcm.properties";
        } else if (file_exists("/etc/dcm.properties")) {
            dcm_file = "/etc/dcm.properties";
        }
        
        if (dcm_file) {
            RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Loading fallback config from %s\n", __FUNCTION__, dcm_file);
            rrd_config_parse_properties(dcm_file, config);
        }
    }
    
    // Log final configuration values
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Configuration loaded - LOG_SERVER: %s, UPLOAD_PROTOCOL: %s, HTTP_UPLOAD_LINK: %s\n",
            __FUNCTION__, 
            config->log_server[0] ? config->log_server : "(empty)",
            config->upload_protocol[0] ? config->upload_protocol : "(empty)",
            config->http_upload_link[0] ? config->http_upload_link : "(empty)");
    
    // Validate essential fields
    if (strlen(config->log_server) == 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: LOG_SERVER is empty after all config attempts!\n", __FUNCTION__);
        return -2;
    }
    
    if (strlen(config->http_upload_link) == 0) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: HTTP_UPLOAD_LINK is empty after all config attempts!\n", __FUNCTION__);
        return -3;
    }
    
    return 0;
}

int rrd_config_parse_properties(const char *filepath, rrd_config_t *config) {
    if (!filepath || !config) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid parameters\n", __FUNCTION__);
        return -1;
    }
    
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Entry - parsing %s\n", __FUNCTION__, filepath);
    
    FILE *f = fopen(filepath, "r");
    if (!f) {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Cannot open file: %s\n", __FUNCTION__, filepath);
        return -2;
    }
    
    int lines_parsed = 0;
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char *key = line;
        char *val = eq + 1;
        trim(key); trim(val);
        
        if (strlen(key) == 0 || strlen(val) == 0) continue;
        
        if (strcmp(key, "LOG_SERVER") == 0) {
            strncpy(config->log_server, val, sizeof(config->log_server)-1);
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Set LOG_SERVER=%s\n", __FUNCTION__, val);
            lines_parsed++;
        }
        else if (strcmp(key, "HTTP_UPLOAD_LINK") == 0) {
            strncpy(config->http_upload_link, val, sizeof(config->http_upload_link)-1);
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Set HTTP_UPLOAD_LINK=%s\n", __FUNCTION__, val);
            lines_parsed++;
        }
        else if (strcmp(key, "UPLOAD_PROTOCOL") == 0) {
            strncpy(config->upload_protocol, val, sizeof(config->upload_protocol)-1);
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Set UPLOAD_PROTOCOL=%s\n", __FUNCTION__, val);
            lines_parsed++;
        }
        else if (strcmp(key, "RDK_PATH") == 0) {
            strncpy(config->rdk_path, val, sizeof(config->rdk_path)-1);
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Set RDK_PATH=%s\n", __FUNCTION__, val);
            lines_parsed++;
        }
        else if (strcmp(key, "LOG_PATH") == 0) {
            strncpy(config->log_path, val, sizeof(config->log_path)-1);
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Set LOG_PATH=%s\n", __FUNCTION__, val);
            lines_parsed++;
        }
        else if (strcmp(key, "BUILD_TYPE") == 0) {
            strncpy(config->build_type, val, sizeof(config->build_type)-1);
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Set BUILD_TYPE=%s\n", __FUNCTION__, val);
            lines_parsed++;
        }
    }
    fclose(f);
    
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Exit - parsed %d properties from %s\n", 
            __FUNCTION__, lines_parsed, filepath);
    return 0;
}

int rrd_config_query_rfc(rrd_config_t *config) {
    if (!config) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid config parameter\n", __FUNCTION__);
        return -1;
    }
    
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Entry - querying RFC parameters\n", __FUNCTION__);
    
    // Check if tr181 tool exists (matching shell script line 84: "if [ -f /usr/bin/tr181 ]; then")
    if (!file_exists("/usr/bin/tr181")) {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: /usr/bin/tr181 not found, skipping RFC query\n", __FUNCTION__);
        return -1;
    }
    
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Found /usr/bin/tr181, proceeding with RFC queries\n", __FUNCTION__);
    
    bool found_any = false;
    char cmd[512];
    char output[512];
    
    // Query LOG_SERVER from RFC (matching shell script lines 86-87)
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Querying RFC parameter: LogServerUrl\n", __FUNCTION__);
    snprintf(cmd, sizeof(cmd), "/usr/bin/tr181 -g Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.LogServerUrl 2>&1");
    if (execute_command(cmd, output, sizeof(output)) == 0 && strlen(output) > 0) {
        trim(output);
        if (strlen(output) > 0 && strcmp(output, "null") != 0) {
            strncpy(config->log_server, output, sizeof(config->log_server)-1);
            RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Using Log Server URL from RFC: %s\n", __FUNCTION__, output);
            found_any = true;
        } else {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: RFC LogServerUrl returned null or empty\n", __FUNCTION__);
        }
    } else {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Failed to query RFC LogServerUrl\n", __FUNCTION__);
    }
    
    // Query HTTP_UPLOAD_LINK from RFC if not already set (matching shell script lines 88-95)
    if (strlen(config->http_upload_link) == 0) {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: HTTP_UPLOAD_LINK not set, querying RFC parameter: SsrUrl\n", __FUNCTION__);
        snprintf(cmd, sizeof(cmd), "/usr/bin/tr181 -g Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.SsrUrl 2>&1");
        if (execute_command(cmd, output, sizeof(output)) == 0 && strlen(output) > 0) {
            trim(output);
            if (strlen(output) > 0 && strcmp(output, "null") != 0) {
                // Append /cgi-bin/S3.cgi to the URL (matching shell script line 93)
                // Use larger buffer to avoid truncation warning (512 + 16 for "/cgi-bin/S3.cgi\0")
                char full_url[600];
                snprintf(full_url, sizeof(full_url), "%s/cgi-bin/S3.cgi", output);
                strncpy(config->http_upload_link, full_url, sizeof(config->http_upload_link)-1);
                RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "%s: Using Upload HttpLink from RFC: %s\n", __FUNCTION__, full_url);
                found_any = true;
            } else {
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: RFC SsrUrl returned null or empty\n", __FUNCTION__);
            }
        } else {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Failed to query RFC SsrUrl\n", __FUNCTION__);
        }
    } else {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: HTTP_UPLOAD_LINK already set, skipping RFC query\n", __FUNCTION__);
    }
    
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Exit - RFC query %s\n", 
            __FUNCTION__, found_any ? "successful" : "failed");
    
    return found_any ? 0 : -2;
}

int rrd_config_parse_dcm_settings(const char *filepath, rrd_config_t *config) {
    if (!filepath || !config) return -1;
    
    FILE *f = fopen(filepath, "r");
    if (!f) {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Cannot open %s\n", __FUNCTION__, filepath);
        return -2;
    }
    
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Parsing DCM settings from %s\n", __FUNCTION__, filepath);
    
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char *key = line;
        char *val = eq + 1;
        trim(key);
        trim(val);
        
        // Remove surrounding quotes from value if present
        size_t val_len = strlen(val);
        if (val_len >= 2 && val[0] == '"' && val[val_len-1] == '"') {
            val[val_len-1] = '\0';
            val++;
        }
        
        // Match shell script parsing for LogUploadSettings fields (lines 102-112)
        if (strcmp(key, "LogUploadSettings:UploadRepository:URL") == 0) {
            if (strlen(val) > 0) {
                strncpy(config->http_upload_link, val, sizeof(config->http_upload_link)-1);
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Set HTTP_UPLOAD_LINK from DCM: %s\n", __FUNCTION__, val);
            }
        }
        else if (strcmp(key, "LogUploadSettings:UploadRepository:uploadProtocol") == 0) {
            if (strlen(val) > 0) {
                strncpy(config->upload_protocol, val, sizeof(config->upload_protocol)-1);
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Set UPLOAD_PROTOCOL from DCM: %s\n", __FUNCTION__, val);
            } else {
                // Default to HTTP if not found (matching shell script lines 111-113)
                strncpy(config->upload_protocol, "HTTP", sizeof(config->upload_protocol)-1);
            }
        }
        // Also handle simple key names for backwards compatibility
        else if (strcmp(key, "LOG_SERVER") == 0 && strlen(val) > 0) {
            strncpy(config->log_server, val, sizeof(config->log_server)-1);
        }
        else if (strcmp(key, "HTTP_UPLOAD_LINK") == 0 && strlen(val) > 0) {
            strncpy(config->http_upload_link, val, sizeof(config->http_upload_link)-1);
        }
        else if (strcmp(key, "UPLOAD_PROTOCOL") == 0 && strlen(val) > 0) {
            strncpy(config->upload_protocol, val, sizeof(config->upload_protocol)-1);
        }
        else if (strcmp(key, "RDK_PATH") == 0 && strlen(val) > 0) {
            strncpy(config->rdk_path, val, sizeof(config->rdk_path)-1);
        }
        else if (strcmp(key, "LOG_PATH") == 0 && strlen(val) > 0) {
            strncpy(config->log_path, val, sizeof(config->log_path)-1);
        }
        else if (strcmp(key, "BUILD_TYPE") == 0 && strlen(val) > 0) {
            strncpy(config->build_type, val, sizeof(config->build_type)-1);
        }
    }
    fclose(f);
    return 0;
}

const char* rrd_config_get_value(const rrd_config_t *config, const char *key) {
    if (!config || !key) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid parameters\n", __FUNCTION__);
        return NULL;
    }
    
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Getting value for key: %s\n", __FUNCTION__, key);
    
    const char *value = NULL;
    if (strcmp(key, "LOG_SERVER") == 0) value = config->log_server;
    else if (strcmp(key, "HTTP_UPLOAD_LINK") == 0) value = config->http_upload_link;
    else if (strcmp(key, "UPLOAD_PROTOCOL") == 0) value = config->upload_protocol;
    else if (strcmp(key, "RDK_PATH") == 0) value = config->rdk_path;
    else if (strcmp(key, "LOG_PATH") == 0) value = config->log_path;
    else if (strcmp(key, "BUILD_TYPE") == 0) value = config->build_type;
    
    if (value) {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Key %s = %s\n", __FUNCTION__, key, value);
    } else {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Unknown key: %s\n", __FUNCTION__, key);
    }
    
    return value;
}

void rrd_config_cleanup(rrd_config_t *config) {
    if (!config) {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "%s: Invalid config parameter\n", __FUNCTION__);
        return;
    }
    
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Cleaning up configuration\n", __FUNCTION__);
    memset(config, 0, sizeof(*config));
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "%s: Configuration cleanup complete\n", __FUNCTION__);
}
