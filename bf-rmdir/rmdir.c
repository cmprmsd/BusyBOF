/*
 * rmdir.c — BOF: remove empty directories
 * Usage: rmdir <dir> [dir2 ...]
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    FILE *pipe = bof_pipe_input();
    if (pipe) {
        char line[PATH_MAX_BB];
        while (fgets(line, sizeof(line), pipe)) {
            line[strcspn(line, "\n")] = '\0';
            if (line[0]) {
                if (rmdir(line) != 0)
                    BeaconPrintf(CALLBACK_ERROR, "rmdir: '%s': %s\n", line, strerror(errno));
                else
                    BeaconPrintf(CALLBACK_OUTPUT, "Removed: %s\n", line);
            }
        }
        fclose(pipe);
        return;
    }

    if (!args || alen <= 0)
        BOF_ERROR("Usage: rmdir <dir> [dir2 ...]");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: rmdir <dir> [dir2 ...]");

    char *saveptr;
    char *tok = strtok_r(argv_str, " ", &saveptr);
    while (tok) {
        if (rmdir(tok) != 0)
            BeaconPrintf(CALLBACK_ERROR, "rmdir: '%s': %s\n", tok, strerror(errno));
        else
            BeaconPrintf(CALLBACK_OUTPUT, "Removed: %s\n", tok);
        tok = strtok_r(NULL, " ", &saveptr);
    }
}
