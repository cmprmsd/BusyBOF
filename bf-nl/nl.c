/*
 * nl.c — BOF: number lines of a file
 * Usage: nl <file>
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    FILE *pipe = bof_pipe_input();
    if (pipe) {
        char line[8192];
        int n = 1;
        while (fgets(line, sizeof(line), pipe)) {
            line[strcspn(line, "\n")] = '\0';
            if (line[0])
                BeaconPrintf(CALLBACK_OUTPUT, "%6d\t%s\n", n++, line);
            else
                BeaconPrintf(CALLBACK_OUTPUT, "\n");
        }
        fclose(pipe);
        return;
    }

    if (!args || alen <= 0) BOF_ERROR("Usage: nl <file>");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *filepath = BeaconDataExtract(&parser, NULL);
    if (!filepath || !*filepath) BOF_ERROR("Usage: nl <file>");

    FILE *fp = fopen(filepath, "r");
    if (!fp) BOF_ERROR("nl: %s: %s", filepath, strerror(errno));

    char line[8192];
    int n = 1;
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (line[0])
            BeaconPrintf(CALLBACK_OUTPUT, "%6d\t%s\n", n++, line);
        else
            BeaconPrintf(CALLBACK_OUTPUT, "\n");
    }
    fclose(fp);
}
