/*
 * rrd_archive.c - Archive Manager (tar writer + gzip via zlib)
 *
 * This implementation replaces shell `tar -czf` invocation with an
 * internal tar writer that streams POSIX tar headers and file data into
 * a gzip stream using zlib (`gz*` API). It supports recursive directory
 * traversal, regular files and directories. Symlinks and special devices
 * are not fully supported in this minimal implementation.
 */

#include "rrd_archive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <zlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/resource.h>

/* POSIX ustar header */
struct posix_header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
};

static void set_octal(char *p, size_t size, unsigned long long val) {
    /* write val as octal with trailing space and null */
    snprintf(p, size, "%0*llo", (int)(size - 1), (unsigned long long)val);
}

static unsigned int calculate_checksum(const unsigned char *block, size_t size) {
    unsigned int sum = 0;
    for (size_t i = 0; i < size; ++i) sum += block[i];
    return sum;
}

static int write_block(gzFile out, const void *buf, size_t size) {
    int written = gzwrite(out, buf, (unsigned)size);
    return (written == (int)size) ? 0 : -1;
}

static int write_zeros(gzFile out, size_t size) {
    char zero[512];
    memset(zero, 0, sizeof(zero));
    while (size > 0) {
        size_t chunk = size > sizeof(zero) ? sizeof(zero) : size;
        if (write_block(out, zero, chunk) != 0) return -1;
        size -= chunk;
    }
    return 0;
}

static int write_tar_header(gzFile out, const char *name, const struct stat *st, char typeflag) {
    struct posix_header hdr;
    memset(&hdr, 0, sizeof(hdr));

    size_t name_len = strlen(name);
    if (name_len <= 100) {
        strncpy(hdr.name, name, sizeof(hdr.name));
    } else if (name_len <= 255) {
        /* split into prefix and name */
        size_t prefix_len = name_len - 100 - 1; /* leave room for null */
        if (prefix_len > sizeof(hdr.prefix)) return -1;
        strncpy(hdr.prefix, name, prefix_len);
        strncpy(hdr.name, name + prefix_len + 1, sizeof(hdr.name));
    } else {
        return -1; /* name too long */
    }

    snprintf(hdr.mode, sizeof(hdr.mode), "%07o", st->st_mode & 07777);
    snprintf(hdr.uid, sizeof(hdr.uid), "%07o", (unsigned)(st->st_uid));
    snprintf(hdr.gid, sizeof(hdr.gid), "%07o", (unsigned)(st->st_gid));
    snprintf(hdr.size, sizeof(hdr.size), "%011llo", (unsigned long long)(typeflag == '5' ? 0ULL : (unsigned long long)st->st_size));
    snprintf(hdr.mtime, sizeof(hdr.mtime), "%011llo", (unsigned long long)st->st_mtime);
    hdr.typeflag = typeflag;
    hdr.magic[0] = 'u'; hdr.magic[1] = 's'; hdr.magic[2] = 't'; hdr.magic[3] = 'a'; hdr.magic[4] = 'r'; hdr.magic[5] = '\0';
    hdr.version[0] = '0'; hdr.version[1] = '0';

    struct passwd *pw = getpwuid(st->st_uid);
    struct group  *gr = getgrgid(st->st_gid);
    if (pw) strncpy(hdr.uname, pw->pw_name, sizeof(hdr.uname)-1);
    if (gr) strncpy(hdr.gname, gr->gr_name, sizeof(hdr.gname)-1);

    /* checksum: set to spaces for calculation */
    memset(hdr.chksum, ' ', sizeof(hdr.chksum));

    /* calculate checksum over the 512-byte header */
    unsigned int csum = calculate_checksum((const unsigned char *)&hdr, sizeof(hdr));
    snprintf(hdr.chksum, sizeof(hdr.chksum), "%06o ", csum);

    if (write_block(out, &hdr, sizeof(hdr)) != 0) return -1;
    return 0;
}

static int write_file_contents(gzFile out, const char *path, const struct stat *st) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    const size_t bufsize = 8192;
    char *buf = (char *)malloc(bufsize);
    if (!buf) { close(fd); return -1; }
    ssize_t r;
    unsigned long long remaining = st->st_size;
    while ((r = read(fd, buf, bufsize)) > 0) {
        if (gzwrite(out, buf, (unsigned)r) != r) { free(buf); close(fd); return -1; }
        remaining -= (unsigned long long)r;
    }
    free(buf);
    close(fd);
    if (r < 0) return -1;
    /* pad to 512 bytes */
    size_t pad = (512 - (st->st_size % 512)) % 512;
    if (pad) {
        if (write_zeros(out, pad) != 0) return -1;
    }
    return 0;
}

static int archive_path_recursive(gzFile out, const char *source_root, const char *current_relpath) {
    char fullpath[4096];
    if (strlen(source_root) + 1 + strlen(current_relpath) + 1 >= sizeof(fullpath)) return -1;
    if (current_relpath[0])
        snprintf(fullpath, sizeof(fullpath), "%s/%s", source_root, current_relpath);
    else
        snprintf(fullpath, sizeof(fullpath), "%s", source_root);

    DIR *d = opendir(fullpath);
    if (!d) {
        /* not a directory -> should be a file handled by caller */
        struct stat st;
        if (lstat(fullpath, &st) != 0) return -1;
        /* write header and file */
        if (write_tar_header(out, current_relpath, &st, '0') != 0) return -1;
        if (S_ISREG(st.st_mode)) {
            if (write_file_contents(out, fullpath, &st) != 0) return -1;
        }
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        char relname[4096];
        if (current_relpath[0]) snprintf(relname, sizeof(relname), "%s/%s", current_relpath, entry->d_name);
        else snprintf(relname, sizeof(relname), "%s", entry->d_name);

        char childpath[4096];
        snprintf(childpath, sizeof(childpath), "%s/%s", source_root, relname);
        struct stat st;
        if (lstat(childpath, &st) != 0) { closedir(d); return -1; }

        if (S_ISDIR(st.st_mode)) {
            /* write directory header (name should end with '/') */
            char dirtarname[4096];
            snprintf(dirtarname, sizeof(dirtarname), "%s/", relname);
            if (write_tar_header(out, dirtarname, &st, '5') != 0) { closedir(d); return -1; }
            /* recurse into directory */
            if (archive_path_recursive(out, source_root, relname) != 0) { closedir(d); return -1; }
        } else if (S_ISREG(st.st_mode)) {
            if (write_tar_header(out, relname, &st, '0') != 0) { closedir(d); return -1; }
            if (write_file_contents(out, childpath, &st) != 0) { closedir(d); return -1; }
        } else {
            /* ignore symlinks and special files for this minimal impl */
            continue;
        }
    }
    closedir(d);
    return 0;
}

int rrd_archive_create(const char *source_dir, const char *working_dir, const char *archive_filename) {
    if (!source_dir || !archive_filename) return -1;

    char outpath[4096];
    if (working_dir && strlen(working_dir) > 0) {
        snprintf(outpath, sizeof(outpath), "%s/%s", working_dir, archive_filename);
    } else {
        snprintf(outpath, sizeof(outpath), "%s", archive_filename);
    }

    gzFile out = gzopen(outpath, "wb");
    if (!out) return -2;

    /* If source_dir itself is a file, archive that single file preserving its name as '.' replacement */
    struct stat stroot;
    if (lstat(source_dir, &stroot) != 0) { gzclose(out); return -2; }

    int rc = 0;
    if (S_ISDIR(stroot.st_mode)) {
        /* archive the contents of the directory (entries relative to root, without leading ./) */
        rc = archive_path_recursive(out, source_dir, "");
    } else if (S_ISREG(stroot.st_mode)) {
        /* single file: use its basename as the entry name */
        const char *base = strrchr(source_dir, '/');
        if (!base) base = source_dir;
        else base++;
        if (write_tar_header(out, base, &stroot, '0') != 0) rc = -1;
        else if (write_file_contents(out, source_dir, &stroot) != 0) rc = -1;
    } else {
        gzclose(out);
        return -2;
    }

    /* two 512-byte blocks of zeros to mark end of archive */
    if (rc == 0) {
        if (write_zeros(out, 512) != 0) rc = -1;
        if (write_zeros(out, 512) != 0) rc = -1;
    }

    gzclose(out);

    struct stat st;
    if (stat(outpath, &st) != 0 || st.st_size == 0) return -3;
    return (rc == 0) ? 0 : -2;
}

// Generate archive filename: <mac>_<issue_type>_<timestamp>.tar.gz
int rrd_archive_generate_filename(const char *mac, const char *issue_type, const char *timestamp, char *filename, size_t size) {
    if (!mac || !issue_type || !timestamp || !filename || size < 16) return -1;
    snprintf(filename, size, "%s_%s_%s.tar.gz", mac, issue_type, timestamp);
    return 0;
}

int rrd_archive_cleanup(const char *archive_path) {
    if (!archive_path) return -1;
    int ret = remove(archive_path);
    return (ret == 0) ? 0 : -2;

}

// Check system CPU usage (Linux: parse /proc/stat)
int rrd_archive_check_cpu_usage(float *cpu_usage) {
    if (!cpu_usage) return -1;
    FILE *f = fopen("/proc/stat", "r");
    if (!f) return -2;
    char buf[256];
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    if (!fgets(buf, sizeof(buf), f)) { fclose(f); return -3; }
    fclose(f);
    int n = sscanf(buf, "cpu %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
    if (n < 4) return -4;
    unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal;
    if (total == 0) return -5;
    *cpu_usage = 100.0f * (float)(total - idle) / (float)total;
    return 0;
}

// Adjust process priority based on CPU usage (lower priority if high usage)
int rrd_archive_adjust_priority(float cpu_usage) {
    int prio = 0;
    if (cpu_usage > 80.0f) prio = 19; // lowest
    else if (cpu_usage > 50.0f) prio = 10;
    else prio = 0; // normal
    int ret = setpriority(PRIO_PROCESS, 0, prio);
    return (ret == 0) ? 0 : -1;
}
