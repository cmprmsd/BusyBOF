/*
 * stat.c — BOF: display file status
 * Usage: stat <file> [file2 ...]
 */
#include "bofdefs.h"

static const char *filetype(mode_t m) {
    if (S_ISREG(m))  return "regular file";
    if (S_ISDIR(m))  return "directory";
    if (S_ISLNK(m))  return "symbolic link";
    if (S_ISCHR(m))  return "character device";
    if (S_ISBLK(m))  return "block device";
    if (S_ISFIFO(m)) return "FIFO/pipe";
    if (S_ISSOCK(m)) return "socket";
    return "unknown";
}

static void stat_one(const char *file) {
    struct stat st;
    if (lstat(file, &st) != 0) {
        BeaconPrintf(CALLBACK_ERROR, "stat: '%s': %s\n", file, strerror(errno));
        return;
    }
    char atime_s[64], mtime_s[64], ctime_s[64];
    strftime(atime_s, sizeof(atime_s), "%Y-%m-%d %H:%M:%S", localtime(&st.st_atime));
    strftime(mtime_s, sizeof(mtime_s), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));
    strftime(ctime_s, sizeof(ctime_s), "%Y-%m-%d %H:%M:%S", localtime(&st.st_ctime));
    BeaconPrintf(CALLBACK_OUTPUT,
        "  File: %s\n"
        "  Size: %-12ld Blocks: %-8ld IO Block: %-6ld %s\n"
        "Device: %lxh  Inode: %-10lu  Links: %ld\n"
        "Access: (%04o)  Uid: (%d)  Gid: (%d)\n"
        "Access: %s\n"
        "Modify: %s\n"
        "Change: %s\n\n",
        file,
        (long)st.st_size, (long)st.st_blocks, (long)st.st_blksize, filetype(st.st_mode),
        (unsigned long)st.st_dev, (unsigned long)st.st_ino, (long)st.st_nlink,
        st.st_mode & 07777, st.st_uid, st.st_gid,
        atime_s, mtime_s, ctime_s);
}

void go(char *args, int alen) {
    FILE *pipe = bof_pipe_input();
    if (pipe) {
        char line[PATH_MAX_BB];
        while (fgets(line, sizeof(line), pipe)) {
            line[strcspn(line, "\n")] = '\0';
            if (line[0]) stat_one(line);
        }
        fclose(pipe);
        return;
    }

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: stat <file> [file2 ...]");

    char *saveptr;
    char *file = strtok_r(argv_str, " ", &saveptr);
    while (file) {
        stat_one(file);
        file = strtok_r(NULL, " ", &saveptr);
    }
}
