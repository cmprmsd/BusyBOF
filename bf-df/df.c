/*
 * df.c — BOF: disk space usage
 * Usage: df [path]
 */
#include "bofdefs.h"
#include <sys/statvfs.h>
#include <mntent.h>

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    BeaconPrintf(CALLBACK_OUTPUT, "%-24s %10s %10s %10s %5s %s\n",
        "Filesystem", "Size", "Used", "Avail", "Use%", "Mounted on");

    if (argv_str && *argv_str) {
        /* Single path */
        struct statvfs sv;
        if (statvfs(argv_str, &sv) != 0)
            BOF_ERROR("df: %s: %s", argv_str, strerror(errno));

        unsigned long long total = (unsigned long long)sv.f_blocks * sv.f_frsize;
        unsigned long long avail = (unsigned long long)sv.f_bavail * sv.f_frsize;
        unsigned long long used = total - avail;
        int pct = total > 0 ? (int)(used * 100 / total) : 0;

        BeaconPrintf(CALLBACK_OUTPUT, "%-24s %8lluM %8lluM %8lluM %4d%% %s\n",
            "-", total/1048576, used/1048576, avail/1048576, pct, argv_str);
        return;
    }

    /* All mount points */
    FILE *mtab = setmntent("/proc/mounts", "r");
    if (!mtab) mtab = setmntent("/etc/mtab", "r");
    if (!mtab) BOF_ERROR("df: cannot read mount table");

    struct mntent *mnt;
    while ((mnt = getmntent(mtab)) != NULL) {
        /* Skip virtual filesystems */
        if (mnt->mnt_fsname[0] != '/') continue;

        struct statvfs sv;
        if (statvfs(mnt->mnt_dir, &sv) != 0) continue;
        if (sv.f_blocks == 0) continue;

        unsigned long long total = (unsigned long long)sv.f_blocks * sv.f_frsize;
        unsigned long long avail = (unsigned long long)sv.f_bavail * sv.f_frsize;
        unsigned long long used = total - avail;
        int pct = total > 0 ? (int)(used * 100 / total) : 0;

        BeaconPrintf(CALLBACK_OUTPUT, "%-24s %8lluM %8lluM %8lluM %4d%% %s\n",
            mnt->mnt_fsname, total/1048576, used/1048576, avail/1048576, pct, mnt->mnt_dir);
    }
    endmntent(mtab);
}
