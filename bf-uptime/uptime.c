/*
 * uptime.c — BOF: print system uptime
 * Usage: uptime
 */
#include "bofdefs.h"
#include <sys/sysinfo.h>

void go(char *args, int alen) {
    struct sysinfo si;
    if (sysinfo(&si) != 0) BOF_ERROR("uptime: %s", strerror(errno));

    long days = si.uptime / 86400;
    long hours = (si.uptime % 86400) / 3600;
    long mins = (si.uptime % 3600) / 60;

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char timebuf[32];
    strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tm);

    BeaconPrintf(CALLBACK_OUTPUT, " %s up %ld day(s), %02ld:%02ld, load: %.2f %.2f %.2f\n",
        timebuf, days, hours, mins,
        (double)si.loads[0] / 65536.0, (double)si.loads[1] / 65536.0, (double)si.loads[2] / 65536.0);
}
