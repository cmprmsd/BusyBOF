/*
 * lsmod.c — BOF: list loaded kernel modules
 * Usage: lsmod
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    FILE *fp = fopen("/proc/modules", "r");
    if (!fp) BOF_ERROR("lsmod: cannot read /proc/modules");

    BeaconPrintf(CALLBACK_OUTPUT, "%-28s %8s  Used by\n", "Module", "Size");

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char name[128], deps[256];
        long size;
        int used;
        deps[0] = '\0';

        if (sscanf(line, "%127s %ld %d %255s", name, &size, &used, deps) >= 3) {
            /* Strip trailing comma from deps list */
            size_t dlen = strlen(deps);
            if (dlen > 0 && deps[dlen - 1] == ',') deps[dlen - 1] = '\0';
            if (strcmp(deps, "-") == 0) deps[0] = '\0';

            BeaconPrintf(CALLBACK_OUTPUT, "%-28s %8ld  %d %s\n",
                name, size, used, deps);
        }
    }
    fclose(fp);
}
