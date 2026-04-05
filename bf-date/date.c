/*
 * date.c — BOF: print current date/time
 * Usage: date [+format]
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char buf[256];

    const char *fmt = "%a %b %e %H:%M:%S %Z %Y"; /* default like GNU date */
    if (argv_str && argv_str[0] == '+')
        fmt = argv_str + 1;

    strftime(buf, sizeof(buf), fmt, tm);
    BeaconPrintf(CALLBACK_OUTPUT, "%s\n", buf);
}
