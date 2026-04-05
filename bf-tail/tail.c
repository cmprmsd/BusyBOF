/*
 * tail.c — BOF: output last part of file
 * Usage: tail [-n lines] <file>
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: tail [-n lines] <file>");

    int nlines = 10;
    char *filepath = NULL;
    char *saveptr;
    char *tok = strtok_r(argv_str, " ", &saveptr);

    while (tok) {
        if (strcmp(tok, "-n") == 0) {
            tok = strtok_r(NULL, " ", &saveptr);
            if (tok) nlines = atoi(tok);
        } else {
            filepath = tok;
        }
        tok = strtok_r(NULL, " ", &saveptr);
    }

    if (!filepath) BOF_ERROR("Usage: tail [-n lines] <file>");

    FILE *fp = fopen(filepath, "r");
    if (!fp) BOF_ERROR("tail: %s: %s", filepath, strerror(errno));

    /* Count total lines */
    char line[4096];
    int total = 0;
    while (fgets(line, sizeof(line), fp)) total++;

    int skip = total - nlines;
    if (skip < 0) skip = 0;

    rewind(fp);
    int cur = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (cur >= skip)
            BeaconPrintf(CALLBACK_OUTPUT, "%s", line);
        cur++;
    }
    fclose(fp);
}
