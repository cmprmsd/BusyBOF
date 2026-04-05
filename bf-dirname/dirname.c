/*
 * dirname.c — BOF: strip filename from path
 * Usage: dirname <path>
 */
#include "bofdefs.h"
#include <libgen.h>

void go(char *args, int alen) {
    FILE *pipe = bof_pipe_input();
    if (pipe) {
        char line[PATH_MAX_BB];
        while (fgets(line, sizeof(line), pipe)) {
            line[strcspn(line, "\n")] = '\0';
            if (line[0]) BeaconPrintf(CALLBACK_OUTPUT, "%s\n", dirname(line));
        }
        fclose(pipe);
        return;
    }

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);
    if (!argv_str || !*argv_str) BOF_ERROR("Usage: dirname <path>");
    BeaconPrintf(CALLBACK_OUTPUT, "%s\n", dirname(argv_str));
}
