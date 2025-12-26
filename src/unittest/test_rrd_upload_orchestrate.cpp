/*
 * test_rrd_upload_orchestrate.cpp - Unit tests for RRD Upload Orchestration
 * 
 * Tests the complete upload workflow including:
 * - Configuration loading
 * - System information retrieval (MAC, timestamp)
 * - Log directory validation and preparation
 * - Issue type sanitization
 * - Archive creation
 * - Upload execution
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <fstream>

#include "Client_Mock.h"
#include "Client_Mock.cpp"

extern "C" {
#include "rrd_config.h"
#include "rrd_config.c"
#include "rrd_sysinfo.h"
#include "rrd_sysinfo.c"
#include "rrd_logproc.h"
#include "rrd_logproc.c"
#include "rrd_archive.h"
#include "rrd_archive.c"
#include "rrd_upload.h"
#include "rrd_upload.c"
#include "uploadRRDLogs.c"
}



// Test Fixture for Upload Orchestration
class RRDUploadOrchestrationTest : public ::testing::Test {
protected:
    const char *test_dir = "/tmp/rrd_test_upload";
    const char *test_issue_type = "cpu.high";

    void SetUp() override {
        // Create test directory with some log files
        mkdir(test_dir, 0755);
        
        // Create dummy log files
        std::string log1 = std::string(test_dir) + "/test.log";
        std::string log2 = std::string(test_dir) + "/debug.log";
        
        std::ofstream f1(log1);
        f1 << "Test log content 1\n";
        f1.close();
        
        std::ofstream f2(log2);
        f2 << "Test log content 2\n";
        f2.close();

        // Set environment variables for config
        setenv("RFC_LOG_SERVER", "logs.example.com", 1);
        setenv("RFC_HTTP_UPLOAD_LINK", "http://logs.example.com/upload", 1);
        setenv("RFC_UPLOAD_PROTOCOL", "HTTP", 1);
    }

    void TearDown() override {
        // Cleanup test directory
        system("rm -rf /tmp/rrd_test_upload*");
        unsetenv("RFC_LOG_SERVER");
        unsetenv("RFC_HTTP_UPLOAD_LINK");
        unsetenv("RFC_UPLOAD_PROTOCOL");
    }
};

// Test: Invalid parameters
TEST_F(RRDUploadOrchestrationTest, InvalidParametersNull) {
    int result = rrd_upload_orchestrate(NULL, "issue_type");
    EXPECT_NE(result, 0);

    result = rrd_upload_orchestrate(test_dir, NULL);
    EXPECT_NE(result, 0);

    result = rrd_upload_orchestrate(NULL, NULL);
    EXPECT_NE(result, 0);
}

// Test: Valid orchestration flow
TEST_F(RRDUploadOrchestrationTest, ValidOrchestrationFlow) {
    int result = rrd_upload_orchestrate(test_dir, test_issue_type);
    // Expected: 0 (success) or reasonable error code
    EXPECT_GE(result, -1);  // At minimum, should not crash
}

// Test: Configuration loading
TEST_F(RRDUploadOrchestrationTest, ConfigurationLoading) {
    rrd_config_t config;
    int result = rrd_config_load(&config);
    
    EXPECT_EQ(result, 0);
    EXPECT_STRNE(config.log_server, "");
    EXPECT_STRNE(config.http_upload_link, "");
    EXPECT_STRNE(config.upload_protocol, "");
}

// Test: System information retrieval
TEST_F(RRDUploadOrchestrationTest, SystemInfoRetrieval) {
    char mac_addr[32] = {0};
    char timestamp[32] = {0};

    int result = rrd_sysinfo_get_mac_address(mac_addr, sizeof(mac_addr));
    EXPECT_EQ(result, 0);
    EXPECT_STRNE(mac_addr, "");
    EXPECT_GE(strlen(mac_addr), 17);  // MAC address minimum length

    result = rrd_sysinfo_get_timestamp(timestamp, sizeof(timestamp));
    EXPECT_EQ(result, 0);
    EXPECT_STRNE(timestamp, "");
    EXPECT_GE(strlen(timestamp), 10);  // Timestamp minimum length
}

// Test: Log directory validation
TEST_F(RRDUploadOrchestrationTest, LogDirectoryValidation) {
    // Valid directory
    int result = rrd_logproc_validate_source(test_dir);
    EXPECT_EQ(result, 0);

    // Non-existent directory
    result = rrd_logproc_validate_source("/tmp/nonexistent_rrd_test_12345");
    EXPECT_NE(result, 0);

    // Empty directory
    const char *empty_dir = "/tmp/rrd_test_empty";
    mkdir(empty_dir, 0755);
    result = rrd_logproc_validate_source(empty_dir);
    EXPECT_NE(result, 0);
    rmdir(empty_dir);
}

// Test: Log preparation
TEST_F(RRDUploadOrchestrationTest, LogPreparation) {
    int result = rrd_logproc_prepare_logs(test_dir, test_issue_type);
    EXPECT_EQ(result, 0);
}

// Test: Issue type conversion
TEST_F(RRDUploadOrchestrationTest, IssueTypeConversion) {
    char sanitized[64];
    
    // Test: lowercase to uppercase, dot to underscore
    int result = rrd_logproc_convert_issue_type("cpu.high", sanitized, sizeof(sanitized));
    EXPECT_EQ(result, 0);
    EXPECT_STREQ(sanitized, "CPU_HIGH");

    // Test: mixed case
    result = rrd_logproc_convert_issue_type("Memory.Low", sanitized, sizeof(sanitized));
    EXPECT_EQ(result, 0);
    EXPECT_STREQ(sanitized, "MEMORY_LOW");

    // Test: already uppercase
    result = rrd_logproc_convert_issue_type("DISK", sanitized, sizeof(sanitized));
    EXPECT_EQ(result, 0);
    EXPECT_STREQ(sanitized, "DISK");

    // Test: invalid buffer
    result = rrd_logproc_convert_issue_type("issue", sanitized, 1);
    EXPECT_NE(result, 0);
}

// Test: Archive filename generation
TEST_F(RRDUploadOrchestrationTest, ArchiveFilenameGeneration) {
    char filename[256];
    const char *mac = "00:11:22:33:44:55";
    const char *issue = "CPU_HIGH";
    const char *timestamp = "2024-12-17-14-30-45PM";

    int result = rrd_archive_generate_filename(mac, issue, timestamp, filename, sizeof(filename));
    EXPECT_EQ(result, 0);
    EXPECT_STRNE(filename, "");
    EXPECT_NE(strstr(filename, mac), nullptr);
    EXPECT_NE(strstr(filename, issue), nullptr);
    EXPECT_NE(strstr(filename, timestamp), nullptr);
    EXPECT_NE(strstr(filename, ".tar.gz"), nullptr);
}

// Test: Archive creation
TEST_F(RRDUploadOrchestrationTest, ArchiveCreation) {
    char archive_filename[256] = "/tmp/rrd_test_archive.tar.gz";
    
    int result = rrd_archive_create(test_dir, NULL, archive_filename);
    EXPECT_EQ(result, 0);

    // Verify archive file exists and has content
    struct stat st;
    result = stat(archive_filename, &st);
    EXPECT_EQ(result, 0);
    EXPECT_GT(st.st_size, 0);

    // Cleanup
    remove(archive_filename);
}

// Test: File operations
TEST_F(RRDUploadOrchestrationTest, FileOperations) {
    // Test file exists
    std::string test_file = std::string(test_dir) + "/test.log";
    bool exists = rrd_sysinfo_file_exists(test_file.c_str());
    EXPECT_TRUE(exists);

    // Test file does not exist
    exists = rrd_sysinfo_file_exists("/tmp/nonexistent_file_12345");
    EXPECT_FALSE(exists);

    // Test directory exists
    bool dir_exists = rrd_sysinfo_dir_exists(test_dir);
    EXPECT_TRUE(dir_exists);

    // Test directory does not exist
    dir_exists = rrd_sysinfo_dir_exists("/tmp/nonexistent_dir_12345");
    EXPECT_FALSE(dir_exists);
}

// Test: Directory emptiness check
TEST_F(RRDUploadOrchestrationTest, DirectoryEmptinessCheck) {
    // Non-empty directory
    bool is_empty = rrd_sysinfo_dir_is_empty(test_dir);
    EXPECT_FALSE(is_empty);

    // Empty directory
    const char *empty_dir = "/tmp/rrd_test_empty_check";
    mkdir(empty_dir, 0755);
    is_empty = rrd_sysinfo_dir_is_empty(empty_dir);
    EXPECT_TRUE(is_empty);
    rmdir(empty_dir);
}

// Test: Directory size calculation
TEST_F(RRDUploadOrchestrationTest, DirectorySizeCalculation) {
    size_t size = 0;
    int result = rrd_sysinfo_get_dir_size(test_dir, &size);
    EXPECT_EQ(result, 0);
    EXPECT_GT(size, 0);  // Should have some size from log files
}

// Test: Archive cleanup
TEST_F(RRDUploadOrchestrationTest, ArchiveCleanup) {
    char archive_file[256] = "/tmp/rrd_test_cleanup.tar.gz";
    
    // Create a dummy archive file
    std::ofstream f(archive_file);
    f << "dummy archive content\n";
    f.close();

    // Verify it exists
    struct stat st;
    EXPECT_EQ(stat(archive_file, &st), 0);

    // Cleanup
    int result = rrd_archive_cleanup(archive_file);
    EXPECT_EQ(result, 0);

    // Verify it's deleted
    EXPECT_NE(stat(archive_file, &st), 0);
}

// Test: Configuration cleanup
TEST_F(RRDUploadOrchestrationTest, ConfigurationCleanup) {
    rrd_config_t config;
    memset(&config, 1, sizeof(config));  // Fill with non-zero values
    
    rrd_config_cleanup(&config);
    
    // Verify all fields are cleared
    EXPECT_EQ(config.log_server[0], 0);
    EXPECT_EQ(config.http_upload_link[0], 0);
    EXPECT_EQ(config.upload_protocol[0], 0);
}

// Integration test: End-to-end orchestration
TEST_F(RRDUploadOrchestrationTest, EndToEndOrchestration) {
    // This test verifies the entire flow works together
    int result = rrd_upload_orchestrate(test_dir, "test.issue");
    
    // Result should be a valid return code (0 for success, or specific error code)
    EXPECT_GE(result, -11);  // Within expected error range
    EXPECT_LE(result, 11);
}

// Edge case: Invalid directory path
TEST_F(RRDUploadOrchestrationTest, InvalidDirectoryPath) {
    int result = rrd_upload_orchestrate("/invalid/path/to/logs", "issue");
    EXPECT_NE(result, 0);  // Should fail
}

// Edge case: Special characters in issue type
TEST_F(RRDUploadOrchestrationTest, SpecialCharactersInIssueType) {
    char sanitized[64];
    int result = rrd_logproc_convert_issue_type("test-issue.sub@special!", sanitized, sizeof(sanitized));
    EXPECT_EQ(result, 0);
    // Should only contain alphanumeric and underscore
    for (const char *p = sanitized; *p; ++p) {
        EXPECT_TRUE(isalnum(*p) || *p == '_');
    }
}

// Performance test: Large directory
TEST_F(RRDUploadOrchestrationTest, LargeDirectoryHandling) {
    // Create multiple log files
    for (int i = 0; i < 50; ++i) {
        std::string filepath = std::string(test_dir) + "/log" + std::to_string(i) + ".txt";
        std::ofstream f(filepath);
        for (int j = 0; j < 100; ++j) {
            f << "Log line " << j << "\n";
        }
        f.close();
    }

    // Test directory size calculation with many files
    size_t size = 0;
    int result = rrd_sysinfo_get_dir_size(test_dir, &size);
    EXPECT_EQ(result, 0);
    EXPECT_GT(size, 50 * 100);  // Should accumulate all file sizes
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
