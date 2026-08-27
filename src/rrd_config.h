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

#ifndef RRD_CONFIG_H
#define RRD_CONFIG_H

#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

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
