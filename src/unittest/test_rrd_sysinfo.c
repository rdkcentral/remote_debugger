/*
 * test_rrd_sysinfo.c - Unit tests for rrd_sysinfo module
 */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../rrd_sysinfo.h"

void test_get_mac_address() {
    char mac[32];
    int ret = rrd_sysinfo_get_mac_address(mac, sizeof(mac));
    printf("test_get_mac_address: ret=%d, mac=%s\n", ret, mac);
    assert(ret == 0);
    assert(strlen(mac) >= 11); // Should be a valid MAC
}

void test_get_timestamp() {
    char ts[32];
    int ret = rrd_sysinfo_get_timestamp(ts, sizeof(ts));
    printf("test_get_timestamp: ret=%d, ts=%s\n", ret, ts);
    assert(ret == 0);
    assert(strlen(ts) > 0);
}

void test_file_exists() {
    FILE *f = fopen("/tmp/testfile.txt", "w");
    assert(f);
    fclose(f);
    assert(rrd_sysinfo_file_exists("/tmp/testfile.txt"));
    remove("/tmp/testfile.txt");
    assert(!rrd_sysinfo_file_exists("/tmp/testfile.txt"));
}

void test_dir_exists_and_empty() {
    system("mkdir -p /tmp/testdir");
    assert(rrd_sysinfo_dir_exists("/tmp/testdir"));
    assert(rrd_sysinfo_dir_is_empty("/tmp/testdir"));
    FILE *f = fopen("/tmp/testdir/file.txt", "w");
    assert(f);
    fclose(f);
    assert(!rrd_sysinfo_dir_is_empty("/tmp/testdir"));
    remove("/tmp/testdir/file.txt");
    rmdir("/tmp/testdir");
}

void test_get_dir_size() {
    system("mkdir -p /tmp/testdir2");
    FILE *f = fopen("/tmp/testdir2/file1.txt", "w");
    fputs("hello", f); fclose(f);
    f = fopen("/tmp/testdir2/file2.txt", "w");
    fputs("world", f); fclose(f);
    size_t size = 0;
    int ret = rrd_sysinfo_get_dir_size("/tmp/testdir2", &size);
    printf("test_get_dir_size: ret=%d, size=%zu\n", ret, size);
    assert(ret == 0);
    assert(size >= 10);
    remove("/tmp/testdir2/file1.txt");
    remove("/tmp/testdir2/file2.txt");
    rmdir("/tmp/testdir2");
}

int main() {
    test_get_mac_address();
    test_get_timestamp();
    test_file_exists();
    test_dir_exists_and_empty();
    test_get_dir_size();
    printf("All rrd_sysinfo tests passed!\n");
    return 0;
}
