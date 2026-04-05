/*
 * ln.c — BOF: create links
 * Usage: ln [-s] <target> <linkname>
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    if (!args || alen <= 0)
        BOF_ERROR("Usage: ln [-s] <target> <linkname>");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: ln [-s] <target> <linkname>");

    int symbolic = 0;
    char *saveptr;
    char *tok = strtok_r(argv_str, " ", &saveptr);
    char *target = NULL, *linkname = NULL;

    while (tok) {
        if (strcmp(tok, "-s") == 0)
            symbolic = 1;
        else if (!target)
            target = tok;
        else
            linkname = tok;
        tok = strtok_r(NULL, " ", &saveptr);
    }

    if (!target || !linkname)
        BOF_ERROR("Usage: ln [-s] <target> <linkname>");

    int ret;
    if (symbolic)
        ret = symlink(target, linkname);
    else
        ret = link(target, linkname);

    if (ret != 0)
        BOF_ERROR("ln: '%s' -> '%s': %s", linkname, target, strerror(errno));

    BeaconPrintf(CALLBACK_OUTPUT, "'%s' -> '%s'%s\n", linkname, target,
                 symbolic ? " [symbolic]" : "");
}
