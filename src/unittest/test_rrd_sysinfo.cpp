/*
 * test_rrd_sysinfo.cpp - Unit tests for rrd_sysinfo module (gtest)
 */
#include <gtest/gtest.h>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include "../rrd_sysinfo.h"

// Helper: create file
static void create_file(const char *path, const char *content = "test") {
    FILE *f = fopen(path, "w");
    ASSERT_TRUE(f != nullptr);
    if (content) fputs(content, f);
    fclose(f);
}

// Helper: remove file
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

TEST(RRDSysInfoTest, GetMacAddress) {
    char mac[32] = {0};
    int ret = rrd_sysinfo_get_mac_address(mac, sizeof(mac));
    EXPECT_EQ(ret, 0);
    EXPECT_GE(strlen(mac), 11u);
}

TEST(RRDSysInfoTest, GetTimestamp) {
    char ts[32] = {0};
    int ret = rrd_sysinfo_get_timestamp(ts, sizeof(ts));
    EXPECT_EQ(ret, 0);
    EXPECT_GT(strlen(ts), 0u);
}

TEST(RRDSysInfoTest, FileExists) {
    const char *file = "/tmp/testfile.txt";
    create_file(file);
    EXPECT_TRUE(rrd_sysinfo_file_exists(file));
    remove_file(file);
    EXPECT_FALSE(rrd_sysinfo_file_exists(file));
}

TEST(RRDSysInfoTest, DirExistsAndEmpty) {
    const char *dir = "/tmp/testdir";
    create_dir(dir);
    EXPECT_TRUE(rrd_sysinfo_dir_exists(dir));
    EXPECT_TRUE(rrd_sysinfo_dir_is_empty(dir));
    char file[64];
    snprintf(file, sizeof(file), "%s/file.txt", dir);
    create_file(file);
    EXPECT_FALSE(rrd_sysinfo_dir_is_empty(dir));
    remove_file(file);
    remove_dir(dir);
}

TEST(RRDSysInfoTest, GetDirSize) {
    const char *dir = "/tmp/testdir2";
    create_dir(dir);
    char file1[64], file2[64];
    snprintf(file1, sizeof(file1), "%s/file1.txt", dir);
    snprintf(file2, sizeof(file2), "%s/file2.txt", dir);
    create_file(file1, "hello");
    create_file(file2, "world");
    size_t size = 0;
    int ret = rrd_sysinfo_get_dir_size(dir, &size);
    EXPECT_EQ(ret, 0);
    EXPECT_GE(size, 10u);
    remove_file(file1);
    remove_file(file2);
    remove_dir(dir);
}

// Add more tests as new APIs are added to rrd_sysinfo
