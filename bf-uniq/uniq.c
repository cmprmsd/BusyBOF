/*
 * uniq.c — BOF: remove duplicate adjacent lines
 * Usage: uniq [-c] <file>
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);
    if (!argv_str || !*argv_str) BOF_ERROR("Usage: uniq [-c] <file>");

    int show_count = 0;
    char *filepath = NULL;
    char *saveptr;
    char *tok = strtok_r(argv_str, " ", &saveptr);
    while (tok) {
        if (strcmp(tok, "-c") == 0) show_count = 1;
        else filepath = tok;
        tok = strtok_r(NULL, " ", &saveptr);
    }
    if (!filepath) BOF_ERROR("Usage: uniq [-c] <file>");

    FILE *fp = fopen(filepath, "r");
    if (!fp) BOF_ERROR("uniq: %s: %s", filepath, strerror(errno));

    char prev[8192] = "", line[8192];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strcmp(line, prev) == 0) {
            count++;
        } else {
            if (count > 0) {
                if (show_count) BeaconPrintf(CALLBACK_OUTPUT, "%7d %s", count, prev);
                else BeaconPrintf(CALLBACK_OUTPUT, "%s", prev);
            }
            strncpy(prev, line, sizeof(prev) - 1);
            prev[sizeof(prev) - 1] = '\0';
            count = 1;
        }
    }
    if (count > 0) {
        if (show_count) BeaconPrintf(CALLBACK_OUTPUT, "%7d %s", count, prev);
        else BeaconPrintf(CALLBACK_OUTPUT, "%s", prev);
    }
    fclose(fp);
}
