# uploadRRDLogs - Low-Level Design Document

## Document Information
- **Component Name:** uploadRRDLogs (C Implementation)
- **Version:** 1.0
- **Date:** December 1, 2025
- **Target Platform:** Embedded Linux Systems

## 1. Executive Summary

This document provides the low-level design (LLD) for the C implementation of uploadRRDLogs. It includes detailed specifications for data structures, function prototypes, algorithms, memory management, and implementation details for each module identified in the High-Level Design.

## 2. Module Specifications

### 2.1 Main Orchestration Module (rrd_main)

#### 2.1.1 Header File: rrd_main.h

```c
#ifndef RRD_MAIN_H
#define RRD_MAIN_H

#include <stdint.h>
#include <stdbool.h>

/* Exit codes */
#define EXIT_SUCCESS              0
#define EXIT_INVALID_ARGS         1
#define EXIT_CONFIG_ERROR         2
#define EXIT_ARCHIVE_ERROR        3
#define EXIT_UPLOAD_ERROR         4
#define EXIT_CLEANUP_WARNING      5

/* Buffer sizes */
#define MAX_PATH_LENGTH           4096
#define MAX_FILENAME_LENGTH       256
#define MAX_ISSUETYPE_LENGTH      64

/* Global context structure */
typedef struct {
    char upload_dir[MAX_PATH_LENGTH];
    char issue_type[MAX_ISSUETYPE_LENGTH];
    char issue_type_upper[MAX_ISSUETYPE_LENGTH];
    char mac_address[32];
    char timestamp[32];
    char archive_filename[MAX_FILENAME_LENGTH];
    char archive_path[MAX_PATH_LENGTH];
    bool cleanup_needed;
    bool upload_success;
} rrd_context_t;

/* Function prototypes */
int main(int argc, char *argv[]);
int rrd_validate_arguments(int argc, char *argv[], rrd_context_t *ctx);
int rrd_orchestrate(rrd_context_t *ctx);
void rrd_cleanup_context(rrd_context_t *ctx);
void rrd_print_usage(const char *program_name);

#endif /* RRD_MAIN_H */
```

#### 2.1.2 Implementation Details

**Function: main**
```c
int main(int argc, char *argv[]) {
    rrd_context_t ctx;
    int result;
    
    /* Initialize context */
    memset(&ctx, 0, sizeof(rrd_context_t));
    
    /* Validate command-line arguments */
    result = rrd_validate_arguments(argc, argv, &ctx);
    if (result != 0) {
        rrd_print_usage(argv[0]);
        return EXIT_INVALID_ARGS;
    }
    
    /* Initialize logging */
    result = rrd_log_init();
    if (result != 0) {
        fprintf(stderr, "Failed to initialize logging\n");
        return EXIT_CONFIG_ERROR;
    }
    
    /* Execute main workflow */
    result = rrd_orchestrate(&ctx);
    
    /* Cleanup */
    rrd_cleanup_context(&ctx);
    rrd_log_cleanup();
    
    return result;
}
```

**Function: rrd_validate_arguments**
```c
int rrd_validate_arguments(int argc, char *argv[], rrd_context_t *ctx) {
    if (argc != 3) {
        return -1;
    }
    
    /* Validate UPLOADDIR */
    if (strlen(argv[1]) >= MAX_PATH_LENGTH) {
        fprintf(stderr, "Error: Upload directory path too long\n");
        return -1;
    }
    
    /* Check for directory traversal attempts */
    if (strstr(argv[1], "..") != NULL) {
        fprintf(stderr, "Error: Invalid path (contains ..)\n");
        return -1;
    }
    
    strncpy(ctx->upload_dir, argv[1], MAX_PATH_LENGTH - 1);
    ctx->upload_dir[MAX_PATH_LENGTH - 1] = '\0';
    
    /* Validate ISSUETYPE */
    if (strlen(argv[2]) >= MAX_ISSUETYPE_LENGTH) {
        fprintf(stderr, "Error: Issue type too long\n");
        return -1;
    }
    
    strncpy(ctx->issue_type, argv[2], MAX_ISSUETYPE_LENGTH - 1);
    ctx->issue_type[MAX_ISSUETYPE_LENGTH - 1] = '\0';
    
    return 0;
}
```

**Function: rrd_orchestrate**
```c
int rrd_orchestrate(rrd_context_t *ctx) {
    rrd_config_t config;
    int result;
    
    /* Load configuration */
    result = rrd_config_load(&config);
    if (result != 0) {
        rrd_log_error("Failed to load configuration");
        return EXIT_CONFIG_ERROR;
    }
    
    /* Get system information */
    result = rrd_sysinfo_get_mac_address(ctx->mac_address, sizeof(ctx->mac_address));
    if (result != 0) {
        rrd_log_error("Failed to retrieve MAC address");
        rrd_config_cleanup(&config);
        return EXIT_CONFIG_ERROR;
    }
    
    result = rrd_sysinfo_get_timestamp(ctx->timestamp, sizeof(ctx->timestamp));
    if (result != 0) {
        rrd_log_error("Failed to generate timestamp");
        rrd_config_cleanup(&config);
        return EXIT_CONFIG_ERROR;
    }
    
    /* Process logs */
    result = rrd_logproc_prepare(&config, ctx);
    if (result != 0) {
        if (result == RRD_LOGPROC_EMPTY) {
            rrd_log_info("Source directory empty, nothing to upload");
            rrd_config_cleanup(&config);
            return EXIT_SUCCESS;
        }
        rrd_log_error("Failed to prepare logs");
        rrd_config_cleanup(&config);
        return EXIT_CONFIG_ERROR;
    }
    
    /* Create archive */
    result = rrd_archive_create(&config, ctx);
    if (result != 0) {
        rrd_log_error("Failed to create archive");
        ctx->cleanup_needed = true;
        rrd_config_cleanup(&config);
        return EXIT_ARCHIVE_ERROR;
    }
    
    ctx->cleanup_needed = true;
    
    /* Upload archive */
    result = rrd_upload_execute(&config, ctx);
    if (result != 0) {
        rrd_log_error("Failed to upload archive");
        ctx->upload_success = false;
        rrd_upload_cleanup_files(ctx);
        rrd_config_cleanup(&config);
        return EXIT_UPLOAD_ERROR;
    }
    
    ctx->upload_success = true;
    
    /* Cleanup files */
    result = rrd_upload_cleanup_files(ctx);
    if (result != 0) {
        rrd_log_warning("Cleanup completed with warnings");
    }
    
    rrd_config_cleanup(&config);
    return EXIT_SUCCESS;
}
```

---

### 2.2 Configuration Manager Module (rrd_config)

#### 2.2.1 Header File: rrd_config.h

```c
#ifndef RRD_CONFIG_H
#define RRD_CONFIG_H

#include <stdbool.h>

#define MAX_CONFIG_VALUE_LENGTH   512
#define MAX_CONFIG_LINE_LENGTH    1024

typedef struct {
    char log_server[MAX_CONFIG_VALUE_LENGTH];
    char http_upload_link[MAX_CONFIG_VALUE_LENGTH];
    char upload_protocol[32];
    char rdk_path[MAX_PATH_LENGTH];
    char log_path[MAX_PATH_LENGTH];
    char build_type[64];
    bool use_rfc_config;
} rrd_config_t;

/* Function prototypes */
int rrd_config_load(rrd_config_t *config);
int rrd_config_parse_properties(const char *filepath, rrd_config_t *config);
int rrd_config_query_rfc(rrd_config_t *config);
int rrd_config_parse_dcm_settings(const char *filepath, rrd_config_t *config);
int rrd_config_parse_dcm_properties(const char *filepath, rrd_config_t *config);
int rrd_config_validate(rrd_config_t *config);
void rrd_config_cleanup(rrd_config_t *config);

/* Helper functions */
static int parse_property_line(const char *line, char *key, char *value, 
                               size_t key_size, size_t value_size);
static void trim_whitespace(char *str);
static void remove_quotes(char *str);

#endif /* RRD_CONFIG_H */
```

#### 2.2.2 Key Data Structures

**Configuration Structure:**
- Stores all configuration parameters
- Fixed-size buffers to avoid dynamic allocation
- Boolean flag for RFC usage tracking

**Property Line Format:**
```
KEY=value
KEY="value with spaces"
# Comment lines (ignored)
```

#### 2.2.3 Implementation Details

**Function: rrd_config_load**
```c
int rrd_config_load(rrd_config_t *config) {
    int result;
    
    /* Initialize with defaults */
    memset(config, 0, sizeof(rrd_config_t));
    strncpy(config->upload_protocol, "HTTP", sizeof(config->upload_protocol) - 1);
    
    /* Load include.properties */
    result = rrd_config_parse_properties("/etc/include.properties", config);
    if (result != 0) {
        rrd_log_warning("Failed to load /etc/include.properties");
    }
    
    /* Load device.properties */
    result = rrd_config_parse_properties("/etc/device.properties", config);
    if (result != 0) {
        rrd_log_warning("Failed to load /etc/device.properties");
    }
    
    /* Check for production override */
    if (strcmp(config->build_type, "prod") == 0 && 
        access("/opt/dcm.properties", F_OK) == 0) {
        rrd_log_info("Production build with DCM override, skipping RFC");
        result = rrd_config_parse_dcm_properties("/opt/dcm.properties", config);
        if (result != 0) {
            rrd_log_error("Failed to load /opt/dcm.properties");
            return -1;
        }
    } else {
        /* Try RFC parameters */
        if (access("/usr/bin/tr181", X_OK) == 0) {
            result = rrd_config_query_rfc(config);
            if (result != 0) {
                rrd_log_warning("RFC query failed, using DCM settings");
            } else {
                config->use_rfc_config = true;
            }
        }
        
        /* Load DCM settings */
        result = rrd_config_parse_dcm_settings("/tmp/DCMSettings.conf", config);
        if (result != 0) {
            rrd_log_warning("DCMSettings.conf not available");
            
            /* Try fallback DCM properties */
            if (access("/opt/dcm.properties", F_OK) == 0) {
                result = rrd_config_parse_dcm_properties("/opt/dcm.properties", config);
            } else if (access("/etc/dcm.properties", F_OK) == 0) {
                result = rrd_config_parse_dcm_properties("/etc/dcm.properties", config);
            }
            
            if (result != 0) {
                rrd_log_error("All DCM configuration sources failed");
                return -1;
            }
        }
    }
    
    /* Validate configuration */
    result = rrd_config_validate(config);
    if (result != 0) {
        rrd_log_error("Configuration validation failed");
        return -1;
    }
    
    rrd_log_info("Configuration loaded: LOG_SERVER=%s, PROTOCOL=%s", 
                 config->log_server, config->upload_protocol);
    
    return 0;
}
```

**Function: rrd_config_parse_properties**
```c
int rrd_config_parse_properties(const char *filepath, rrd_config_t *config) {
    FILE *fp;
    char line[MAX_CONFIG_LINE_LENGTH];
    char key[256], value[MAX_CONFIG_VALUE_LENGTH];
    int line_num = 0;
    
    fp = fopen(filepath, "r");
    if (fp == NULL) {
        return -1;
    }
    
    while (fgets(line, sizeof(line), fp) != NULL) {
        line_num++;
        
        /* Skip empty lines and comments */
        trim_whitespace(line);
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }
        
        /* Parse key=value */
        if (parse_property_line(line, key, value, sizeof(key), sizeof(value)) != 0) {
            rrd_log_warning("Malformed line %d in %s", line_num, filepath);
            continue;
        }
        
        /* Store values based on key */
        if (strcmp(key, "RDK_PATH") == 0) {
            strncpy(config->rdk_path, value, sizeof(config->rdk_path) - 1);
        } else if (strcmp(key, "LOG_PATH") == 0) {
            strncpy(config->log_path, value, sizeof(config->log_path) - 1);
        } else if (strcmp(key, "BUILD_TYPE") == 0) {
            strncpy(config->build_type, value, sizeof(config->build_type) - 1);
        } else if (strcmp(key, "LOG_SERVER") == 0) {
            strncpy(config->log_server, value, sizeof(config->log_server) - 1);
        } else if (strcmp(key, "HTTP_UPLOAD_LINK") == 0) {
            strncpy(config->http_upload_link, value, sizeof(config->http_upload_link) - 1);
        } else if (strcmp(key, "UPLOAD_PROTOCOL") == 0) {
            strncpy(config->upload_protocol, value, sizeof(config->upload_protocol) - 1);
        }
    }
    
    fclose(fp);
    return 0;
}
```

**Function: rrd_config_query_rfc**
```c
int rrd_config_query_rfc(rrd_config_t *config) {
    char cmd[512];
    char output[MAX_CONFIG_VALUE_LENGTH];
    FILE *fp;
    
    /* Query LogServerUrl */
    snprintf(cmd, sizeof(cmd), 
             "/usr/bin/tr181 -g Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.LogServerUrl 2>&1");
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        return -1;
    }
    
    if (fgets(output, sizeof(output), fp) != NULL) {
        trim_whitespace(output);
        if (output[0] != '\0' && strstr(output, "error") == NULL) {
            strncpy(config->log_server, output, sizeof(config->log_server) - 1);
        }
    }
    pclose(fp);
    
    /* Query SsrUrl */
    snprintf(cmd, sizeof(cmd),
             "/usr/bin/tr181 -g Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.LogUpload.SsrUrl 2>&1");
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        return -1;
    }
    
    if (fgets(output, sizeof(output), fp) != NULL) {
        trim_whitespace(output);
        if (output[0] != '\0' && strstr(output, "error") == NULL) {
            /* Append /cgi-bin/S3.cgi if not present */
            if (strstr(output, "S3.cgi") == NULL) {
                snprintf(config->http_upload_link, sizeof(config->http_upload_link),
                         "%s/cgi-bin/S3.cgi", output);
            } else {
                strncpy(config->http_upload_link, output, 
                       sizeof(config->http_upload_link) - 1);
            }
        }
    }
    pclose(fp);
    
    return 0;
}
```

**Function: parse_property_line**
```c
static int parse_property_line(const char *line, char *key, char *value,
                               size_t key_size, size_t value_size) {
    const char *equals_pos;
    size_t key_len, value_len;
    
    /* Find equals sign */
    equals_pos = strchr(line, '=');
    if (equals_pos == NULL) {
        return -1;
    }
    
    /* Extract key */
    key_len = equals_pos - line;
    if (key_len >= key_size) {
        return -1;
    }
    
    strncpy(key, line, key_len);
    key[key_len] = '\0';
    trim_whitespace(key);
    
    /* Extract value */
    value_len = strlen(equals_pos + 1);
    if (value_len >= value_size) {
        return -1;
    }
    
    strncpy(value, equals_pos + 1, value_size - 1);
    value[value_size - 1] = '\0';
    trim_whitespace(value);
    remove_quotes(value);
    
    return 0;
}
```

---

### 2.3 System Info Provider Module (rrd_sysinfo)

#### 2.3.1 Header File: rrd_sysinfo.h

```c
#ifndef RRD_SYSINFO_H
#define RRD_SYSINFO_H

#include <stdbool.h>
#include <sys/types.h>

/* Function prototypes */
int rrd_sysinfo_get_mac_address(char *mac_addr, size_t size);
int rrd_sysinfo_get_timestamp(char *timestamp, size_t size);
bool rrd_sysinfo_file_exists(const char *filepath);
bool rrd_sysinfo_dir_exists(const char *dirpath);
bool rrd_sysinfo_dir_is_empty(const char *dirpath);
int rrd_sysinfo_get_dir_size(const char *dirpath, uint64_t *size);
int rrd_sysinfo_get_available_space(const char *path, uint64_t *available);

/* Helper functions */
static int read_mac_from_sysfs(const char *interface, char *mac_addr, size_t size);
static int read_mac_from_ioctl(const char *interface, char *mac_addr, size_t size);
static int read_mac_from_utils(char *mac_addr, size_t size);

#endif /* RRD_SYSINFO_H */
```

#### 2.3.2 Implementation Details

**Function: rrd_sysinfo_get_mac_address**
```c
int rrd_sysinfo_get_mac_address(char *mac_addr, size_t size) {
    const char *interfaces[] = {"eth0", "wlan0", "erouter0", NULL};
    int i;
    int result;
    
    if (size < 18) { /* Minimum size for XX:XX:XX:XX:XX:XX\0 */
        return -1;
    }
    
    /* Method 1: Try reading from sysfs */
    for (i = 0; interfaces[i] != NULL; i++) {
        result = read_mac_from_sysfs(interfaces[i], mac_addr, size);
        if (result == 0) {
            return 0;
        }
    }
    
    /* Method 2: Try ioctl */
    for (i = 0; interfaces[i] != NULL; i++) {
        result = read_mac_from_ioctl(interfaces[i], mac_addr, size);
        if (result == 0) {
            return 0;
        }
    }
    
    /* Method 3: Try utils.sh getMacAddressOnly (compatibility) */
    result = read_mac_from_utils(mac_addr, size);
    if (result == 0) {
        return 0;
    }
    
    return -1;
}

static int read_mac_from_sysfs(const char *interface, char *mac_addr, size_t size) {
    char path[256];
    FILE *fp;
    
    snprintf(path, sizeof(path), "/sys/class/net/%s/address", interface);
    
    fp = fopen(path, "r");
    if (fp == NULL) {
        return -1;
    }
    
    if (fgets(mac_addr, size, fp) == NULL) {
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    
    /* Remove newline */
    mac_addr[strcspn(mac_addr, "\n")] = '\0';
    
    /* Validate format */
    if (strlen(mac_addr) != 17) {
        return -1;
    }
    
    return 0;
}

static int read_mac_from_ioctl(const char *interface, char *mac_addr, size_t size) {
    int sock;
    struct ifreq ifr;
    unsigned char *hwaddr;
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return -1;
    }
    
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
    
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        close(sock);
        return -1;
    }
    
    close(sock);
    
    hwaddr = (unsigned char *)ifr.ifr_hwaddr.sa_data;
    snprintf(mac_addr, size, "%02x:%02x:%02x:%02x:%02x:%02x",
             hwaddr[0], hwaddr[1], hwaddr[2], 
             hwaddr[3], hwaddr[4], hwaddr[5]);
    
    return 0;
}
```

**Function: rrd_sysinfo_get_timestamp**
```c
int rrd_sysinfo_get_timestamp(char *timestamp, size_t size) {
    time_t now;
    struct tm *tm_info;
    char am_pm[3];
    int hour_12;
    
    if (size < 32) {
        return -1;
    }
    
    time(&now);
    tm_info = localtime(&now);
    
    if (tm_info == NULL) {
        return -1;
    }
    
    /* Convert 24-hour to 12-hour format */
    hour_12 = tm_info->tm_hour % 12;
    if (hour_12 == 0) hour_12 = 12;
    
    /* Determine AM/PM */
    strncpy(am_pm, (tm_info->tm_hour >= 12) ? "PM" : "AM", sizeof(am_pm));
    
    /* Format: YYYY-MM-DD-HH-MM-SSAM/PM */
    snprintf(timestamp, size, "%04d-%02d-%02d-%02d-%02d-%02d%s",
             tm_info->tm_year + 1900,
             tm_info->tm_mon + 1,
             tm_info->tm_mday,
             hour_12,
             tm_info->tm_min,
             tm_info->tm_sec,
             am_pm);
    
    return 0;
}
```

**Function: rrd_sysinfo_dir_is_empty**
```c
bool rrd_sysinfo_dir_is_empty(const char *dirpath) {
    DIR *dir;
    struct dirent *entry;
    int count = 0;
    
    dir = opendir(dirpath);
    if (dir == NULL) {
        return true; /* Treat error as empty */
    }
    
    while ((entry = readdir(dir)) != NULL) {
        /* Skip . and .. */
        if (strcmp(entry->d_name, ".") == 0 || 
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        count++;
        break; /* Found at least one entry */
    }
    
    closedir(dir);
    return (count == 0);
}
```

---

### 2.4 Log Processing Module (rrd_logproc)

#### 2.4.1 Header File: rrd_logproc.h

```c
#ifndef RRD_LOGPROC_H
#define RRD_LOGPROC_H

#include "rrd_config.h"
#include "rrd_main.h"

/* Error codes */
#define RRD_LOGPROC_SUCCESS       0
#define RRD_LOGPROC_EMPTY         1
#define RRD_LOGPROC_ERROR         -1

/* Function prototypes */
int rrd_logproc_prepare(const rrd_config_t *config, rrd_context_t *ctx);
int rrd_logproc_validate_source(const char *source_dir);
int rrd_logproc_convert_issue_type(const char *input, char *output, size_t size);
int rrd_logproc_handle_live_logs(const rrd_config_t *config, const char *source_dir);

#endif /* RRD_LOGPROC_H */
```

#### 2.4.2 Implementation Details

**Function: rrd_logproc_prepare**
```c
int rrd_logproc_prepare(const rrd_config_t *config, rrd_context_t *ctx) {
    int result;
    
    /* Validate source directory */
    result = rrd_logproc_validate_source(ctx->upload_dir);
    if (result != 0) {
        if (result == RRD_LOGPROC_EMPTY) {
            rrd_log_info("%s is empty, nothing to upload", ctx->upload_dir);
            return RRD_LOGPROC_EMPTY;
        }
        return RRD_LOGPROC_ERROR;
    }
    
    /* Convert issue type to uppercase */
    result = rrd_logproc_convert_issue_type(ctx->issue_type, 
                                           ctx->issue_type_upper,
                                           sizeof(ctx->issue_type_upper));
    if (result != 0) {
        rrd_log_error("Failed to convert issue type");
        return RRD_LOGPROC_ERROR;
    }
    
    /* Handle special case: LOGUPLOAD_ENABLE */
    if (strcmp(ctx->issue_type_upper, "LOGUPLOAD_ENABLE") == 0) {
        result = rrd_logproc_handle_live_logs(config, ctx->upload_dir);
        if (result != 0) {
            rrd_log_warning("Failed to include live logs, continuing without them");
            /* Non-fatal, continue */
        }
    }
    
    rrd_log_info("Log processing complete for issue type: %s", ctx->issue_type_upper);
    return RRD_LOGPROC_SUCCESS;
}
```

**Function: rrd_logproc_validate_source**
```c
int rrd_logproc_validate_source(const char *source_dir) {
    struct stat st;
    
    /* Check directory exists */
    if (stat(source_dir, &st) != 0) {
        rrd_log_error("Source directory does not exist: %s", source_dir);
        return RRD_LOGPROC_ERROR;
    }
    
    /* Check it's a directory */
    if (!S_ISDIR(st.st_mode)) {
        rrd_log_error("Path is not a directory: %s", source_dir);
        return RRD_LOGPROC_ERROR;
    }
    
    /* Check readable */
    if (access(source_dir, R_OK) != 0) {
        rrd_log_error("Source directory not readable: %s", source_dir);
        return RRD_LOGPROC_ERROR;
    }
    
    /* Check not empty */
    if (rrd_sysinfo_dir_is_empty(source_dir)) {
        return RRD_LOGPROC_EMPTY;
    }
    
    return RRD_LOGPROC_SUCCESS;
}
```

**Function: rrd_logproc_convert_issue_type**
```c
int rrd_logproc_convert_issue_type(const char *input, char *output, size_t size) {
    size_t i;
    size_t len;
    
    len = strlen(input);
    if (len >= size) {
        return -1;
    }
    
    for (i = 0; i < len; i++) {
        output[i] = toupper((unsigned char)input[i]);
    }
    output[len] = '\0';
    
    return 0;
}
```

**Function: rrd_logproc_handle_live_logs**
```c
int rrd_logproc_handle_live_logs(const rrd_config_t *config, const char *source_dir) {
    char src_path[MAX_PATH_LENGTH];
    char dst_path[MAX_PATH_LENGTH];
    
    /* Build source path: /tmp/rrd/RRD_LIVE_LOGS.tar.gz */
    snprintf(src_path, sizeof(src_path), "/tmp/rrd/RRD_LIVE_LOGS.tar.gz");
    
    /* Check if file exists */
    if (access(src_path, F_OK) != 0) {
        rrd_log_info("Live logs file not found: %s", src_path);
        return -1;
    }
    
    /* Build destination path */
    snprintf(dst_path, sizeof(dst_path), "%s/RRD_LIVE_LOGS.tar.gz", source_dir);
    
    /* Move file */
    if (rename(src_path, dst_path) != 0) {
        rrd_log_warning("Failed to move live logs: %s", strerror(errno));
        return -1;
    }
    
    rrd_log_info("Live logs included in upload");
    return 0;
}
```

---

### 2.5 Archive Manager Module (rrd_archive)

#### 2.5.1 Header File: rrd_archive.h

```c
#ifndef RRD_ARCHIVE_H
#define RRD_ARCHIVE_H

#include "rrd_config.h"
#include "rrd_main.h"

/* Archive buffer size */
#define ARCHIVE_BUFFER_SIZE       8192

/* Function prototypes */
int rrd_archive_create(const rrd_config_t *config, rrd_context_t *ctx);
int rrd_archive_generate_filename(const char *mac, const char *issue_type,
                                  const char *timestamp, char *filename, size_t size);
int rrd_archive_verify(const char *archive_path);

/* Implementation options */
#ifdef HAVE_LIBARCHIVE
int rrd_archive_create_with_libarchive(const char *source_dir, 
                                       const char *archive_path);
#endif

int rrd_archive_create_with_tar(const char *source_dir, 
                                const char *archive_path);

#endif /* RRD_ARCHIVE_H */
```

#### 2.5.2 Implementation Details

**Function: rrd_archive_create**
```c
int rrd_archive_create(const rrd_config_t *config, rrd_context_t *ctx) {
    int result;
    char working_dir[] = "/tmp/rrd";
    uint64_t available_space;
    
    /* Generate archive filename */
    result = rrd_archive_generate_filename(ctx->mac_address,
                                          ctx->issue_type_upper,
                                          ctx->timestamp,
                                          ctx->archive_filename,
                                          sizeof(ctx->archive_filename));
    if (result != 0) {
        rrd_log_error("Failed to generate archive filename");
        return -1;
    }
    
    /* Build full archive path */
    snprintf(ctx->archive_path, sizeof(ctx->archive_path),
             "%s/%s", working_dir, ctx->archive_filename);
    
    /* Change to working directory */
    if (chdir(working_dir) != 0) {
        rrd_log_error("Failed to change to working directory: %s", working_dir);
        return -1;
    }
    
    /* Check available disk space */
    result = rrd_sysinfo_get_available_space("/tmp", &available_space);
    if (result == 0) {
        uint64_t dir_size;
        result = rrd_sysinfo_get_dir_size(ctx->upload_dir, &dir_size);
        if (result == 0) {
            uint64_t required = dir_size + (dir_size / 5); /* 20% overhead */
            if (available_space < required) {
                rrd_log_error("Insufficient disk space: %llu available, %llu required",
                             available_space, required);
                return -1;
            }
        }
    }
    
    rrd_log_info("Creating archive: %s", ctx->archive_filename);
    
#ifdef HAVE_LIBARCHIVE
    result = rrd_archive_create_with_libarchive(ctx->upload_dir, ctx->archive_path);
#else
    result = rrd_archive_create_with_tar(ctx->upload_dir, ctx->archive_path);
#endif
    
    if (result != 0) {
        rrd_log_error("Archive creation failed");
        return -1;
    }
    
    /* Verify archive */
    result = rrd_archive_verify(ctx->archive_path);
    if (result != 0) {
        rrd_log_error("Archive verification failed");
        return -1;
    }
    
    rrd_log_info("Archive created successfully");
    return 0;
}
```

**Function: rrd_archive_generate_filename**
```c
int rrd_archive_generate_filename(const char *mac, const char *issue_type,
                                  const char *timestamp, char *filename, size_t size) {
    char sanitized_mac[32];
    size_t i, j;
    int result;
    
    /* Sanitize MAC address: remove colons */
    j = 0;
    for (i = 0; mac[i] != '\0' && j < sizeof(sanitized_mac) - 1; i++) {
        if (mac[i] != ':') {
            sanitized_mac[j++] = mac[i];
        }
    }
    sanitized_mac[j] = '\0';
    
    /* Generate filename: {MAC}_{ISSUETYPE}_{TIMESTAMP}_RRD_DEBUG_LOGS.tgz */
    result = snprintf(filename, size, "%s_%s_%s_RRD_DEBUG_LOGS.tgz",
                     sanitized_mac, issue_type, timestamp);
    
    if (result < 0 || result >= size) {
        return -1;
    }
    
    return 0;
}
```

**Function: rrd_archive_create_with_tar (Fallback Method)**
```c
int rrd_archive_create_with_tar(const char *source_dir, const char *archive_path) {
    char cmd[MAX_PATH_LENGTH * 2 + 128];
    int result;
    char log_redirect[MAX_PATH_LENGTH + 32];
    
    /* Build tar command */
    snprintf(log_redirect, sizeof(log_redirect), 
             "2>> %s/remote-debugger.log", getenv("LOG_PATH") ?: "/opt/logs");
    
    snprintf(cmd, sizeof(cmd),
             "tar -zcf %s -C %s . %s",
             archive_path, source_dir, log_redirect);
    
    rrd_log_info("Executing: %s", cmd);
    
    /* Execute tar command */
    result = system(cmd);
    
    if (result != 0) {
        rrd_log_error("tar command failed with exit code: %d", WEXITSTATUS(result));
        /* Remove partial archive */
        unlink(archive_path);
        return -1;
    }
    
    return 0;
}
```

**Function: rrd_archive_verify**
```c
int rrd_archive_verify(const char *archive_path) {
    struct stat st;
    
    /* Check file exists */
    if (stat(archive_path, &st) != 0) {
        rrd_log_error("Archive file does not exist: %s", archive_path);
        return -1;
    }
    
    /* Check file size > 0 */
    if (st.st_size == 0) {
        rrd_log_error("Archive file is empty: %s", archive_path);
        return -1;
    }
    
    rrd_log_info("Archive verified: %s (%lld bytes)", archive_path, (long long)st.st_size);
    return 0;
}
```

---

### 2.6 Upload Manager Module (rrd_upload)

#### 2.6.1 Header File: rrd_upload.h

```c
#ifndef RRD_UPLOAD_H
#define RRD_UPLOAD_H

#include "rrd_config.h"
#include "rrd_main.h"

/* Upload constants */
#define UPLOAD_LOCK_FILE          "/tmp/.log-upload.pid"
#define MAX_UPLOAD_ATTEMPTS       10
#define UPLOAD_WAIT_SECONDS       60

/* Function prototypes */
int rrd_upload_execute(const rrd_config_t *config, rrd_context_t *ctx);
int rrd_upload_wait_for_lock(int max_attempts, int wait_seconds);
int rrd_upload_invoke_stb_script(const rrd_config_t *config, 
                                 const char *archive_filename);
int rrd_upload_cleanup_files(rrd_context_t *ctx);
int rrd_upload_remove_directory_recursive(const char *dirpath);

#endif /* RRD_UPLOAD_H */
```

#### 2.6.2 Implementation Details

**Function: rrd_upload_execute**
```c
int rrd_upload_execute(const rrd_config_t *config, rrd_context_t *ctx) {
    int result;
    char working_dir[] = "/tmp/rrd";
    
    rrd_log_info("Starting upload process for %s", ctx->archive_filename);
    
    /* Wait for upload lock */
    result = rrd_upload_wait_for_lock(MAX_UPLOAD_ATTEMPTS, UPLOAD_WAIT_SECONDS);
    if (result != 0) {
        rrd_log_error("Upload lock timeout after %d attempts", MAX_UPLOAD_ATTEMPTS);
        return -1;
    }
    
    /* Change to working directory */
    if (chdir(working_dir) != 0) {
        rrd_log_error("Failed to change to working directory: %s", working_dir);
        return -1;
    }
    
    /* Invoke upload script */
    result = rrd_upload_invoke_stb_script(config, ctx->archive_filename);
    if (result != 0) {
        rrd_log_error("Upload failed with code: %d", result);
        return -1;
    }
    
    rrd_log_info("Upload completed successfully");
    return 0;
}
```

**Function: rrd_upload_wait_for_lock**
```c
int rrd_upload_wait_for_lock(int max_attempts, int wait_seconds) {
    int attempt;
    
    for (attempt = 1; attempt <= max_attempts; attempt++) {
        /* Check if lock file exists */
        if (access(UPLOAD_LOCK_FILE, F_OK) != 0) {
            /* Lock is free */
            rrd_log_info("Upload lock is free, proceeding");
            return 0;
        }
        
        /* Lock exists */
        if (attempt < max_attempts) {
            rrd_log_info("Upload lock detected, waiting %d seconds (attempt %d/%d)",
                        wait_seconds, attempt, max_attempts);
            sleep(wait_seconds);
        }
    }
    
    /* Max attempts exceeded */
    return -1;
}
```

**Function: rrd_upload_invoke_stb_script**
```c
int rrd_upload_invoke_stb_script(const rrd_config_t *config, 
                                 const char *archive_filename) {
    pid_t pid;
    int status;
    char *args[12];
    char script_path[MAX_PATH_LENGTH];
    
    /* Build script path */
    snprintf(script_path, sizeof(script_path), 
             "%s/uploadSTBLogs.sh", config->rdk_path);
    
    /* Verify script exists and is executable */
    if (access(script_path, X_OK) != 0) {
        rrd_log_error("uploadSTBLogs.sh not found or not executable: %s", script_path);
        return -1;
    }
    
    rrd_log_info("Executing: %s %s 1 1 0 %s %s 0 1 %s",
                 script_path, config->log_server, config->upload_protocol,
                 config->http_upload_link, archive_filename);
    
    /* Fork and execute */
    pid = fork();
    if (pid < 0) {
        rrd_log_error("Fork failed: %s", strerror(errno));
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        args[0] = script_path;
        args[1] = (char *)config->log_server;
        args[2] = "1";
        args[3] = "1";
        args[4] = "0";
        args[5] = (char *)config->upload_protocol;
        args[6] = (char *)config->http_upload_link;
        args[7] = "0";
        args[8] = "1";
        args[9] = (char *)archive_filename;
        args[10] = NULL;
        
        execv(script_path, args);
        
        /* If execv returns, it failed */
        rrd_log_error("execv failed: %s", strerror(errno));
        _exit(127);
    }
    
    /* Parent process: wait for child */
    if (waitpid(pid, &status, 0) < 0) {
        rrd_log_error("waitpid failed: %s", strerror(errno));
        return -1;
    }
    
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if (exit_code != 0) {
            rrd_log_error("uploadSTBLogs.sh exited with code: %d", exit_code);
            return exit_code;
        }
    } else {
        rrd_log_error("uploadSTBLogs.sh terminated abnormally");
        return -1;
    }
    
    return 0;
}
```

**Function: rrd_upload_cleanup_files**
```c
int rrd_upload_cleanup_files(rrd_context_t *ctx) {
    int result = 0;
    int errors = 0;
    
    rrd_log_info("Starting cleanup operations");
    
    /* Remove archive file */
    if (ctx->archive_path[0] != '\0') {
        if (unlink(ctx->archive_path) != 0) {
            if (errno != ENOENT) {
                rrd_log_warning("Failed to remove archive: %s", strerror(errno));
                errors++;
            } else {
                rrd_log_info("Archive file not found (already removed)");
            }
        } else {
            rrd_log_info("Archive removed: %s", ctx->archive_path);
        }
    }
    
    /* Remove source directory recursively */
    if (ctx->upload_dir[0] != '\0') {
        result = rrd_upload_remove_directory_recursive(ctx->upload_dir);
        if (result != 0) {
            rrd_log_warning("Failed to remove source directory: %s", ctx->upload_dir);
            errors++;
        } else {
            rrd_log_info("Source directory removed: %s", ctx->upload_dir);
        }
    }
    
    if (ctx->upload_success) {
        rrd_log_info("RRD %s Debug Information Report upload Success", 
                    ctx->issue_type_upper);
    } else {
        rrd_log_error("RRD %s Debug Information Report upload Failed!!!", 
                     ctx->issue_type_upper);
    }
    
    return (errors > 0) ? -1 : 0;
}
```

**Function: rrd_upload_remove_directory_recursive**
```c
int rrd_upload_remove_directory_recursive(const char *dirpath) {
    DIR *dir;
    struct dirent *entry;
    char path[MAX_PATH_LENGTH];
    struct stat st;
    int errors = 0;
    
    dir = opendir(dirpath);
    if (dir == NULL) {
        if (errno == ENOENT) {
            return 0; /* Directory doesn't exist, treat as success */
        }
        return -1;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        /* Skip . and .. */
        if (strcmp(entry->d_name, ".") == 0 || 
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        /* Build full path */
        snprintf(path, sizeof(path), "%s/%s", dirpath, entry->d_name);
        
        /* Get file info */
        if (lstat(path, &st) != 0) {
            errors++;
            continue;
        }
        
        /* Recursively remove directories */
        if (S_ISDIR(st.st_mode)) {
            if (rrd_upload_remove_directory_recursive(path) != 0) {
                errors++;
            }
        } else {
            /* Remove file */
            if (unlink(path) != 0) {
                errors++;
            }
        }
    }
    
    closedir(dir);
    
    /* Remove the directory itself */
    if (rmdir(dirpath) != 0) {
        errors++;
    }
    
    return (errors > 0) ? -1 : 0;
}
```

---

### 2.7 Logging Module (rrd_log)

#### 2.7.1 Header File: rrd_log.h

```c
#ifndef RRD_LOG_H
#define RRD_LOG_H

#include <stdarg.h>

/* Log levels */
typedef enum {
    RRD_LOG_DEBUG,
    RRD_LOG_INFO,
    RRD_LOG_WARNING,
    RRD_LOG_ERROR
} rrd_log_level_t;

/* Function prototypes */
int rrd_log_init(void);
void rrd_log_write(rrd_log_level_t level, const char *format, ...);
void rrd_log_cleanup(void);

/* Convenience macros */
#define rrd_log_debug(...)   rrd_log_write(RRD_LOG_DEBUG, __VA_ARGS__)
#define rrd_log_info(...)    rrd_log_write(RRD_LOG_INFO, __VA_ARGS__)
#define rrd_log_warning(...) rrd_log_write(RRD_LOG_WARNING, __VA_ARGS__)
#define rrd_log_error(...)   rrd_log_write(RRD_LOG_ERROR, __VA_ARGS__)

#endif /* RRD_LOG_H */
```

#### 2.7.2 Implementation Details

**Global State:**
```c
static FILE *log_file = NULL;
static const char *log_level_strings[] = {
    "[DEBUG]",
    "[INFO]",
    "[WARN]",
    "[ERROR]"
};
```

**Function: rrd_log_init**
```c
int rrd_log_init(void) {
    const char *log_path;
    char log_file_path[MAX_PATH_LENGTH];
    
    /* Get log path from environment or use default */
    log_path = getenv("LOG_PATH");
    if (log_path == NULL) {
        log_path = "/opt/logs";
    }
    
    /* Build log file path */
    snprintf(log_file_path, sizeof(log_file_path),
             "%s/remote-debugger.log", log_path);
    
    /* Open log file in append mode */
    log_file = fopen(log_file_path, "a");
    if (log_file == NULL) {
        fprintf(stderr, "Failed to open log file: %s\n", log_file_path);
        return -1;
    }
    
    /* Set line buffering */
    setvbuf(log_file, NULL, _IOLBF, 0);
    
    return 0;
}
```

**Function: rrd_log_write**
```c
void rrd_log_write(rrd_log_level_t level, const char *format, ...) {
    va_list args;
    char timestamp[32];
    time_t now;
    struct tm *tm_info;
    
    if (log_file == NULL) {
        return; /* Logging not initialized */
    }
    
    /* Generate timestamp */
    time(&now);
    tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d-%H-%M-%S", tm_info);
    
    /* Write log entry */
    fprintf(log_file, "%s: uploadRRDLogs: %s ", 
            timestamp, log_level_strings[level]);
    
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    fprintf(log_file, "\n");
    
    /* Flush for critical messages */
    if (level >= RRD_LOG_WARNING) {
        fflush(log_file);
    }
}
```

**Function: rrd_log_cleanup**
```c
void rrd_log_cleanup(void) {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}
```

---

## 3. Memory Management Strategy

### 3.1 Stack vs Heap Allocation

**Stack Allocation (Preferred):**
- All fixed-size buffers (paths, config values)
- Context structures
- Temporary variables

**Heap Allocation (Minimal, if needed):**
- Large temporary buffers (only if > 8KB)
- Dynamic data structures (avoid if possible)

### 3.2 Buffer Sizes

```c
#define MAX_PATH_LENGTH           4096   /* System PATH_MAX */
#define MAX_FILENAME_LENGTH       256    /* System NAME_MAX */
#define MAX_CONFIG_VALUE_LENGTH   512    /* Config values */
#define MAX_CONFIG_LINE_LENGTH    1024   /* Config file lines */
#define ARCHIVE_BUFFER_SIZE       8192   /* Archive I/O buffer */
```

### 3.3 Memory Safety

**String Operations:**
- Always use `strncpy()` with size limits
- Always null-terminate strings
- Use `snprintf()` instead of `sprintf()`

**Buffer Overflow Prevention:**
```c
/* Good practice */
strncpy(dest, src, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';

/* Better practice */
snprintf(dest, sizeof(dest), "%s", src);
```

---

## 4. Error Handling Implementation

### 4.1 Error Code Definitions

```c
/* Module-specific error codes */
#define RRD_SUCCESS               0
#define RRD_ERROR_GENERIC         -1
#define RRD_ERROR_INVALID_ARG     -2
#define RRD_ERROR_NOT_FOUND       -3
#define RRD_ERROR_PERMISSION      -4
#define RRD_ERROR_NO_SPACE        -5
#define RRD_ERROR_TIMEOUT         -6
```

### 4.2 Error Handling Pattern

```c
int function_with_error_handling(void) {
    int result = RRD_SUCCESS;
    resource_t *res = NULL;
    
    /* Attempt operation */
    res = allocate_resource();
    if (res == NULL) {
        rrd_log_error("Resource allocation failed");
        result = RRD_ERROR_GENERIC;
        goto cleanup;
    }
    
    /* Perform work */
    result = do_work(res);
    if (result != RRD_SUCCESS) {
        rrd_log_error("Work failed: %d", result);
        goto cleanup;
    }
    
cleanup:
    /* Always clean up resources */
    if (res != NULL) {
        free_resource(res);
    }
    
    return result;
}
```

---

## 5. Build System Integration

### 5.1 Autotools Configuration

**configure.ac additions:**
```autoconf
# Check for libarchive (optional)
AC_CHECK_LIB([archive], [archive_write_new], 
             [HAVE_LIBARCHIVE=1], [HAVE_LIBARCHIVE=0])
AC_SUBST(HAVE_LIBARCHIVE)

# Check for required headers
AC_CHECK_HEADERS([sys/ioctl.h net/if.h])

# Check for tr181 tool
AC_CHECK_PROG([TR181], [tr181], [yes], [no], [/usr/bin])
```

**Makefile.am:**
```makefile
bin_PROGRAMS = uploadRRDLogs

uploadRRDLogs_SOURCES = \
    src/rrd_main.c \
    src/rrd_config.c \
    src/rrd_sysinfo.c \
    src/rrd_logproc.c \
    src/rrd_archive.c \
    src/rrd_upload.c \
    src/rrd_log.c

uploadRRDLogs_CFLAGS = \
    -Wall -Wextra -Werror \
    -std=c99 \
    -Os \
    -I$(top_srcdir)/include

uploadRRDLogs_LDFLAGS = 

if HAVE_LIBARCHIVE
uploadRRDLogs_LDFLAGS += -larchive
uploadRRDLogs_CFLAGS += -DHAVE_LIBARCHIVE
endif
```

### 5.2 Compilation Flags

**Development Build:**
```bash
./configure CFLAGS="-g -O0 -DDEBUG"
make
```

**Production Build:**
```bash
./configure CFLAGS="-Os -DNDEBUG" --enable-strip
make
```

**Cross-Compilation:**
```bash
./configure --host=arm-linux-gnueabihf \
            CC=arm-linux-gnueabihf-gcc \
            CFLAGS="-Os -march=armv7-a"
make
```

---

## 6. Testing Strategy

### 6.1 Unit Test Structure

**Test Framework:** Google Test

**Test Files:**
- `test/test_rrd_config.cpp`
- `test/test_rrd_sysinfo.cpp`
- `test/test_rrd_logproc.cpp`
- `test/test_rrd_archive.cpp`
- `test/test_rrd_upload.cpp`

**Example Unit Test:**
```cpp
TEST(RRDConfigTest, ParsePropertiesSuccess) {
    rrd_config_t config;
    int result;
    
    memset(&config, 0, sizeof(config));
    
    result = rrd_config_parse_properties("test/data/sample.properties", &config);
    
    EXPECT_EQ(result, 0);
    EXPECT_STREQ(config.rdk_path, "/lib/rdk");
    EXPECT_STREQ(config.log_path, "/opt/logs");
}

TEST(RRDSysInfoTest, GetMACAddress) {
    char mac[32];
    int result;
    
    result = rrd_sysinfo_get_mac_address(mac, sizeof(mac));
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(strlen(mac), 17); /* XX:XX:XX:XX:XX:XX */
}
```

### 6.2 Integration Tests

**Test Scenarios:**
1. End-to-end successful upload
2. Configuration fallback chain
3. Empty directory handling
4. Upload lock contention
5. Archive creation for various sizes
6. Upload failure and cleanup

**Integration Test Script:**
```bash
#!/bin/bash
# integration_test.sh

# Setup
export RDK_PATH=/lib/rdk
export LOG_PATH=/tmp/test_logs
mkdir -p /tmp/test_logs /tmp/rrd_test /tmp/rrd

# Create test files
for i in {1..10}; do
    echo "Test log $i" > /tmp/rrd_test/log$i.txt
done

# Run uploadRRDLogs
./uploadRRDLogs /tmp/rrd_test test_issue

# Verify
if [ $? -eq 0 ]; then
    echo "Test PASSED"
else
    echo "Test FAILED"
fi

# Cleanup
rm -rf /tmp/test_logs /tmp/rrd_test
```

---

## 7. Performance Optimization

### 7.1 I/O Optimization

**Buffered I/O:**
```c
/* Use appropriate buffer sizes */
#define IO_BUFFER_SIZE 8192

char buffer[IO_BUFFER_SIZE];
setvbuf(fp, buffer, _IOFBF, IO_BUFFER_SIZE);
```

**Minimize System Calls:**
```c
/* Bad: Multiple small reads */
while (fgets(line, 256, fp)) {
    process_line(line);
}

/* Good: Buffered reading */
char buffer[8192];
size_t bytes_read;
while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
    process_buffer(buffer, bytes_read);
}
```

### 7.2 String Operation Optimization

**Avoid Repeated strlen:**
```c
/* Bad */
for (i = 0; i < strlen(str); i++) {
    process_char(str[i]);
}

/* Good */
len = strlen(str);
for (i = 0; i < len; i++) {
    process_char(str[i]);
}
```

### 7.3 Memory Access Patterns

**Sequential Access:**
- Process arrays/buffers sequentially
- Cache-friendly access patterns

---

## 8. Security Implementation

### 8.1 Input Validation

**Path Validation:**
```c
int validate_path(const char *path) {
    /* Check for NULL */
    if (path == NULL) {
        return -1;
    }
    
    /* Check length */
    if (strlen(path) >= MAX_PATH_LENGTH) {
        return -1;
    }
    
    /* Check for directory traversal */
    if (strstr(path, "..") != NULL) {
        rrd_log_error("Path contains '..': %s", path);
        return -1;
    }
    
    /* Check for absolute path if required */
    if (path[0] != '/') {
        rrd_log_error("Path must be absolute: %s", path);
        return -1;
    }
    
    return 0;
}
```

**String Sanitization:**
```c
int sanitize_issue_type(char *issue_type) {
    size_t i;
    
    for (i = 0; issue_type[i] != '\0'; i++) {
        /* Allow only alphanumeric and underscore */
        if (!isalnum((unsigned char)issue_type[i]) && 
            issue_type[i] != '_') {
            issue_type[i] = '_';
        }
    }
    
    return 0;
}
```

### 8.2 Secure File Operations

**Create files with restrictive permissions:**
```c
int create_secure_file(const char *filepath) {
    int fd;
    mode_t old_umask;
    
    /* Set umask to create file with 0600 permissions */
    old_umask = umask(0077);
    
    fd = open(filepath, O_CREAT | O_WRONLY | O_EXCL, 0600);
    
    /* Restore umask */
    umask(old_umask);
    
    return fd;
}
```

---

## Document Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | December 1, 2025 | GitHub Copilot | Initial LLD document |
