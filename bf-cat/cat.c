/*
 * cat.c — BOF: concatenate and print files
 * Usage: cat <file> [file2 ...]
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);

    char *argv_str = BeaconDataExtract(&parser, NULL);
    if (!argv_str || !*argv_str) {
        BOF_ERROR("Usage: cat <file> [file2 ...]");
    }

    /* Split on spaces for multiple files */
    char *saveptr;
    char *file = strtok_r(argv_str, " ", &saveptr);

    while (file) {
        FILE *fp = fopen(file, "r");
        if (!fp) {
            BeaconPrintf(CALLBACK_ERROR, "cat: %s: %s", file, strerror(errno));
            file = strtok_r(NULL, " ", &saveptr);
            continue;
        }

        char buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf) - 1, fp)) > 0) {
            buf[n] = '\0';
            BeaconPrintf(CALLBACK_OUTPUT, "%s", buf);
        }
        fclose(fp);
        file = strtok_r(NULL, " ", &saveptr);
    }
}
