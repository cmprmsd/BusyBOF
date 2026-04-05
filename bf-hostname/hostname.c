/*
 * hostname.c — BOF: print hostname
 * Usage: hostname
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    char buf[256];
    if (gethostname(buf, sizeof(buf)) != 0)
        BOF_ERROR("hostname: %s", strerror(errno));
    BeaconPrintf(CALLBACK_OUTPUT, "%s\n", buf);
}
