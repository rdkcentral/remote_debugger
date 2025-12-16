/*
 * rrd_config.h - Configuration Manager (skeleton)
 */
#ifndef RRD_CONFIG_H
#define RRD_CONFIG_H

#include <stdbool.h>

// Configuration structure
typedef struct {
    char log_server[256];
    char http_upload_link[512];
    char upload_protocol[16];
    char rdk_path[256];
    char log_path[256];
    char build_type[32];
    bool use_rfc_config;
} rrd_config_t;

// Function prototypes
int rrd_config_load(rrd_config_t *config);
int rrd_config_parse_properties(const char *filepath, rrd_config_t *config);
int rrd_config_query_rfc(rrd_config_t *config);
int rrd_config_parse_dcm_settings(const char *filepath, rrd_config_t *config);
const char* rrd_config_get_value(const rrd_config_t *config, const char *key);
void rrd_config_cleanup(rrd_config_t *config);

#endif // RRD_CONFIG_H
