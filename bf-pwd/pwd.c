/*
 * pwd.c — BOF: print working directory
 * Usage: pwd
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    char buf[PATH_MAX_BB];
    if (!getcwd(buf, sizeof(buf)))
        BOF_ERROR("pwd: %s", strerror(errno));
    BeaconPrintf(CALLBACK_OUTPUT, "%s\n", buf);
}
