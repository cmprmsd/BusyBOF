/*
 * strings.c — BOF: find printable strings in binary files
 * Usage: strings [-n minlen] <file>
 */
#include "bofdefs.h"
#include <ctype.h>

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: strings [-n minlen] <file>");

    int minlen = 4;
    char *filepath = NULL;
    char *saveptr;
    char *tok = strtok_r(argv_str, " ", &saveptr);

    while (tok) {
        if (strcmp(tok, "-n") == 0) {
            tok = strtok_r(NULL, " ", &saveptr);
            if (tok) minlen = atoi(tok);
        } else {
            filepath = tok;
        }
        tok = strtok_r(NULL, " ", &saveptr);
    }

    if (!filepath) BOF_ERROR("Usage: strings [-n minlen] <file>");

    FILE *fp = fopen(filepath, "rb");
    if (!fp) BOF_ERROR("strings: %s: %s", filepath, strerror(errno));

    char buf[8192];
    int pos = 0;
    int c;

    while ((c = fgetc(fp)) != EOF) {
        if (isprint(c) || c == '\t') {
            if (pos < (int)sizeof(buf) - 1)
                buf[pos++] = (char)c;
        } else {
            if (pos >= minlen) {
                buf[pos] = '\0';
                BeaconPrintf(CALLBACK_OUTPUT, "%s\n", buf);
            }
            pos = 0;
        }
    }
    /* Flush remaining */
    if (pos >= minlen) {
        buf[pos] = '\0';
        BeaconPrintf(CALLBACK_OUTPUT, "%s\n", buf);
    }
    fclose(fp);
}
