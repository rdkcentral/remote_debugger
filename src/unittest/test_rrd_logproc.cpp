/*
 * test_rrd_logproc.cpp - Unit tests for rrd_logproc module
 */
#include <gtest/gtest.h>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include "../rrd_logproc.h"

// Helper: create a file with content
static void create_file(const char *path, const char *content = "test") {
    FILE *f = fopen(path, "w");
    ASSERT_TRUE(f != nullptr);
    if (content) fputs(content, f);
    fclose(f);
}

// Helper: remove file if exists
static void remove_file(const char *path) {
    remove(path);
}

// Helper: create directory
static void create_dir(const char *path) {
    mkdir(path, 0777);
}

// Helper: remove directory
static void remove_dir(const char *path) {
    rmdir(path);
}

// Test rrd_logproc_get_log_file_size
TEST(RRDLogProcTest, GetLogFileSize) {
    const char *file = "/tmp/testlogfile.txt";
    create_file(file, "1234567890");
    size_t size = 0;
    int ret = rrd_logproc_get_log_file_size(file, &size);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(size, 10u);
    remove_file(file);
}

// Test rrd_logproc_get_log_file_size for non-existent file
TEST(RRDLogProcTest, GetLogFileSizeNonExistent) {
    size_t size = 0;
    int ret = rrd_logproc_get_log_file_size("/tmp/doesnotexist.txt", &size);
    EXPECT_NE(ret, 0);
}

// Test rrd_logproc_rotate_log_file
TEST(RRDLogProcTest, RotateLogFile) {
    const char *src = "/tmp/testlogsrc.txt";
    const char *dst = "/tmp/testlogdst.txt";
    create_file(src, "logdata");
    remove_file(dst);
    int ret = rrd_logproc_rotate_log_file(src, dst);
    EXPECT_EQ(ret, 0);
    FILE *f = fopen(dst, "r");
    ASSERT_TRUE(f != nullptr);
    char buf[32] = {0};
    fgets(buf, sizeof(buf), f);
    fclose(f);
    EXPECT_STREQ(buf, "logdata");
    remove_file(src);
    remove_file(dst);
}

// Test rrd_logproc_rotate_log_file with missing src
TEST(RRDLogProcTest, RotateLogFileSrcMissing) {
    const char *src = "/tmp/missinglogsrc.txt";
    const char *dst = "/tmp/testlogdst2.txt";
    remove_file(src);
    remove_file(dst);
    int ret = rrd_logproc_rotate_log_file(src, dst);
    EXPECT_NE(ret, 0);
}

// Test rrd_logproc_clear_log_file
TEST(RRDLogProcTest, ClearLogFile) {
    const char *file = "/tmp/testlogclear.txt";
    create_file(file, "somedata");
    int ret = rrd_logproc_clear_log_file(file);
    EXPECT_EQ(ret, 0);
    FILE *f = fopen(file, "r");
    ASSERT_TRUE(f != nullptr);
    char buf[8] = {0};
    size_t n = fread(buf, 1, sizeof(buf), f);
    fclose(f);
    EXPECT_EQ(n, 0u); // Should be empty
    remove_file(file);
}

// Test rrd_logproc_clear_log_file with missing file
TEST(RRDLogProcTest, ClearLogFileMissing) {
    const char *file = "/tmp/missinglogclear.txt";
    remove_file(file);
    int ret = rrd_logproc_clear_log_file(file);
    EXPECT_NE(ret, 0);
}

// Add more tests as new APIs are added to rrd_logproc
