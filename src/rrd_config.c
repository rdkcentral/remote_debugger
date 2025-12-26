/*
 * rrd_config.c - Configuration Manager (skeleton)
 */
#include "rrd_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Helper: trim whitespace
static void trim(char *str) {
    if (!str) return;
    char *end;
    // Trim leading
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;
    // Trim trailing
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) *end-- = 0;
}

int rrd_config_load(rrd_config_t *config) {
    if (!config) return -1;
    memset(config, 0, sizeof(*config));
    config->use_rfc_config = false;
    // 1. Parse properties file
    int prop_ok = rrd_config_parse_properties("/etc/include.properties", config);
    // 2. Try RFC (if available)
    int rfc_ok = rrd_config_query_rfc(config);
    if (rfc_ok == 0) config->use_rfc_config = true;
    // 3. Parse DCM settings (may override some fields)
    int dcm_ok = rrd_config_parse_dcm_settings("/tmp/DCMSettings.conf", config);
    // If all failed, error
    if (prop_ok != 0 && rfc_ok != 0 && dcm_ok != 0) return -2;
    return 0;
}

int rrd_config_parse_properties(const char *filepath, rrd_config_t *config) {
    if (!filepath || !config) return -1;
    FILE *f = fopen(filepath, "r");
    if (!f) return -2;
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char *key = line;
        char *val = eq + 1;
        trim(key); trim(val);
        if (strcmp(key, "LOG_SERVER") == 0) strncpy(config->log_server, val, sizeof(config->log_server)-1);
        else if (strcmp(key, "HTTP_UPLOAD_LINK") == 0) strncpy(config->http_upload_link, val, sizeof(config->http_upload_link)-1);
        else if (strcmp(key, "UPLOAD_PROTOCOL") == 0) strncpy(config->upload_protocol, val, sizeof(config->upload_protocol)-1);
        else if (strcmp(key, "RDK_PATH") == 0) strncpy(config->rdk_path, val, sizeof(config->rdk_path)-1);
        else if (strcmp(key, "LOG_PATH") == 0) strncpy(config->log_path, val, sizeof(config->log_path)-1);
        else if (strcmp(key, "BUILD_TYPE") == 0) strncpy(config->build_type, val, sizeof(config->build_type)-1);
    }
    fclose(f);
    return 0;
}

int rrd_config_query_rfc(rrd_config_t *config) {
    // Stub: In real system, query RFC via RBus or similar API
    // Here, simulate with environment variables for test/demo
    if (!config) return -1;
    const char *val;
    val = getenv("RFC_LOG_SERVER");
    if (val) strncpy(config->log_server, val, sizeof(config->log_server)-1);
    val = getenv("RFC_HTTP_UPLOAD_LINK");
    if (val) strncpy(config->http_upload_link, val, sizeof(config->http_upload_link)-1);
    val = getenv("RFC_UPLOAD_PROTOCOL");
    if (val) strncpy(config->upload_protocol, val, sizeof(config->upload_protocol)-1);
    val = getenv("RFC_RDK_PATH");
    if (val) strncpy(config->rdk_path, val, sizeof(config->rdk_path)-1);
    val = getenv("RFC_LOG_PATH");
    if (val) strncpy(config->log_path, val, sizeof(config->log_path)-1);
    val = getenv("RFC_BUILD_TYPE");
    if (val) strncpy(config->build_type, val, sizeof(config->build_type)-1);
    // If at least one RFC value found, return 0
    if (getenv("RFC_LOG_SERVER") || getenv("RFC_HTTP_UPLOAD_LINK") || getenv("RFC_UPLOAD_PROTOCOL") || getenv("RFC_RDK_PATH") || getenv("RFC_LOG_PATH") || getenv("RFC_BUILD_TYPE"))
        return 0;
    return -2;
}

int rrd_config_parse_dcm_settings(const char *filepath, rrd_config_t *config) {
    if (!filepath || !config) return -1;
    FILE *f = fopen(filepath, "r");
    if (!f) return -2;
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char *key = line;
        char *val = eq + 1;
        trim(key); trim(val);
        if (strcmp(key, "LOG_SERVER") == 0) strncpy(config->log_server, val, sizeof(config->log_server)-1);
        else if (strcmp(key, "HTTP_UPLOAD_LINK") == 0) strncpy(config->http_upload_link, val, sizeof(config->http_upload_link)-1);
        else if (strcmp(key, "UPLOAD_PROTOCOL") == 0) strncpy(config->upload_protocol, val, sizeof(config->upload_protocol)-1);
        else if (strcmp(key, "RDK_PATH") == 0) strncpy(config->rdk_path, val, sizeof(config->rdk_path)-1);
        else if (strcmp(key, "LOG_PATH") == 0) strncpy(config->log_path, val, sizeof(config->log_path)-1);
        else if (strcmp(key, "BUILD_TYPE") == 0) strncpy(config->build_type, val, sizeof(config->build_type)-1);
    }
    fclose(f);
    return 0;
}

const char* rrd_config_get_value(const rrd_config_t *config, const char *key) {
    if (!config || !key) return NULL;
    if (strcmp(key, "LOG_SERVER") == 0) return config->log_server;
    if (strcmp(key, "HTTP_UPLOAD_LINK") == 0) return config->http_upload_link;
    if (strcmp(key, "UPLOAD_PROTOCOL") == 0) return config->upload_protocol;
    if (strcmp(key, "RDK_PATH") == 0) return config->rdk_path;
    if (strcmp(key, "LOG_PATH") == 0) return config->log_path;
    if (strcmp(key, "BUILD_TYPE") == 0) return config->build_type;
    return NULL;
}

void rrd_config_cleanup(rrd_config_t *config) {
    if (!config) return;
    memset(config, 0, sizeof(*config));
}
