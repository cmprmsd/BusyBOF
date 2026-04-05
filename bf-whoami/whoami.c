/*
 * whoami.c — BOF: print effective user name
 * Usage: whoami
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    struct passwd *pw = getpwuid(geteuid());
    if (pw)
        BeaconPrintf(CALLBACK_OUTPUT, "%s\n", pw->pw_name);
    else
        BeaconPrintf(CALLBACK_OUTPUT, "uid=%d\n", geteuid());
}
