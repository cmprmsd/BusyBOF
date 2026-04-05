/*
 * mv.c — BOF: move/rename files
 * Usage: mv <source> <dest>
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    if (!args || alen <= 0)
        BOF_ERROR("Usage: mv <source> <dest>");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *src = BeaconDataExtract(&parser, NULL);
    char *dst = BeaconDataExtract(&parser, NULL);

    if (!src || !*src || !dst || !*dst)
        BOF_ERROR("Usage: mv <source> <dest>");

    /* If dst is a directory, append source basename */
    struct stat st;
    char finaldst[PATH_MAX_BB];
    if (stat(dst, &st) == 0 && S_ISDIR(st.st_mode)) {
        const char *base = strrchr(src, '/');
        base = base ? base + 1 : src;
        snprintf(finaldst, sizeof(finaldst), "%s/%s", dst, base);
    } else {
        strncpy(finaldst, dst, sizeof(finaldst) - 1);
        finaldst[sizeof(finaldst) - 1] = '\0';
    }

    if (rename(src, finaldst) != 0) {
        BOF_ERROR("mv: cannot move '%s' to '%s': %s", src, finaldst, strerror(errno));
    }

    BeaconPrintf(CALLBACK_OUTPUT, "'%s' -> '%s'\n", src, finaldst);
}
