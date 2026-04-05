/*
 * mount.c — BOF: list mount points (read-only, no actual mounting)
 * Usage: mount
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    FILE *fp = fopen("/proc/mounts", "r");
    if (!fp) fp = fopen("/etc/mtab", "r");
    if (!fp) BOF_ERROR("mount: cannot read mount table");

    char line[512];
    while (fgets(line, sizeof(line), fp))
        BeaconPrintf(CALLBACK_OUTPUT, "%s", line);
    fclose(fp);
}
