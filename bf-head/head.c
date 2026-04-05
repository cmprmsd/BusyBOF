/*
 * head.c — BOF: output first part of file
 * Usage: head [-n lines] <file>
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: head [-n lines] <file>");

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

    if (!filepath) BOF_ERROR("Usage: head [-n lines] <file>");

    FILE *fp = fopen(filepath, "r");
    if (!fp) BOF_ERROR("head: %s: %s", filepath, strerror(errno));

    char line[4096];
    int count = 0;
    while (count < nlines && fgets(line, sizeof(line), fp)) {
        BeaconPrintf(CALLBACK_OUTPUT, "%s", line);
        count++;
    }
    fclose(fp);
}
