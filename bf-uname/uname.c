/*
 * uname.c — BOF: print system information
 * Usage: uname [-a]
 */
#include "bofdefs.h"
#include <sys/utsname.h>

void go(char *args, int alen) {
    struct utsname u;
    if (uname(&u) != 0) BOF_ERROR("uname: %s", strerror(errno));

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    if (argv_str && strstr(argv_str, "-a")) {
        BeaconPrintf(CALLBACK_OUTPUT, "%s %s %s %s %s\n",
            u.sysname, u.nodename, u.release, u.version, u.machine);
    } else {
        BeaconPrintf(CALLBACK_OUTPUT, "%s\n", u.sysname);
    }
}
