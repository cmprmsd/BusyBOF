/*
 * paste.c — BOF: merge lines of files side-by-side
 * Usage: paste <file1> <file2> [file3 ...]
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    if (!args || alen <= 0) BOF_ERROR("Usage: paste <file1> <file2> [file3 ...]");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);
    if (!argv_str || !*argv_str) BOF_ERROR("Usage: paste <file1> <file2> [file3 ...]");

    char *files[32];
    int nfiles = 0;
    char *saveptr;
    char *tok = strtok_r(argv_str, " ", &saveptr);
    while (tok && nfiles < 32) {
        files[nfiles++] = tok;
        tok = strtok_r(NULL, " ", &saveptr);
    }
    if (nfiles < 2) BOF_ERROR("Usage: paste <file1> <file2> [file3 ...]");

    FILE *fps[32];
    for (int i = 0; i < nfiles; i++) {
        fps[i] = fopen(files[i], "r");
        if (!fps[i]) BOF_ERROR("paste: %s: %s", files[i], strerror(errno));
    }

    int active = nfiles;
    while (active > 0) {
        active = 0;
        for (int i = 0; i < nfiles; i++) {
            if (i > 0) BeaconPrintf(CALLBACK_OUTPUT, "\t");
            if (fps[i]) {
                char line[4096];
                if (fgets(line, sizeof(line), fps[i])) {
                    line[strcspn(line, "\n")] = '\0';
                    BeaconPrintf(CALLBACK_OUTPUT, "%s", line);
                    active++;
                } else {
                    fclose(fps[i]); fps[i] = NULL;
                }
            }
        }
        if (active > 0) BeaconPrintf(CALLBACK_OUTPUT, "\n");
    }
}
