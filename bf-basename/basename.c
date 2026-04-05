/*
 * basename.c — BOF: strip directory from path
 * Usage: basename <path> [suffix]
 */
#include "bofdefs.h"
#include <libgen.h>

void go(char *args, int alen) {
    /* Pipe input: iterate over lines from previous stage */
    FILE *pipe = bof_pipe_input();
    if (pipe) {
        char line[PATH_MAX_BB];
        while (fgets(line, sizeof(line), pipe)) {
            line[strcspn(line, "\n")] = '\0';
            if (line[0]) {
                char *b = basename(line);
                BeaconPrintf(CALLBACK_OUTPUT, "%s\n", b);
            }
        }
        fclose(pipe);
        return;
    }

    if (!args || alen <= 0) BOF_ERROR("Usage: basename <path> [suffix]");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *path = BeaconDataExtract(&parser, NULL);
    char *suffix = BeaconDataExtract(&parser, NULL);

    if (!path || !*path) BOF_ERROR("Usage: basename <path> [suffix]");

    /* Treat empty optional suffix as NULL */
    if (suffix && !*suffix) suffix = NULL;

    char *base = basename(path);
    if (suffix) {
        size_t blen = strlen(base), slen = strlen(suffix);
        if (blen > slen && strcmp(base + blen - slen, suffix) == 0)
            base[blen - slen] = '\0';
    }
    BeaconPrintf(CALLBACK_OUTPUT, "%s\n", base);
}
