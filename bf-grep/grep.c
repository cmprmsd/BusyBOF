/*
 * grep.c — BOF: search file contents
 * Usage: grep [-inv] <pattern> <file> [file2 ...]
 * Basic substring matching (-i case insensitive, -n line numbers, -v invert)
 */
#include "bofdefs.h"
#include <ctype.h>

static void strtolower(char *s) {
    for (; *s; s++) *s = (char)tolower((unsigned char)*s);
}

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: grep [-inv] <pattern> <file> [file2 ...]");

    int ignore_case = 0, show_num = 0, invert = 0;
    char *pattern = NULL;
    char *files[256];
    int nfiles = 0;

    char *saveptr;
    char *tok = strtok_r(argv_str, " ", &saveptr);
    while (tok) {
        if (tok[0] == '-' && tok[1] && !pattern) {
            for (int i = 1; tok[i]; i++) {
                if (tok[i] == 'i') ignore_case = 1;
                if (tok[i] == 'n') show_num = 1;
                if (tok[i] == 'v') invert = 1;
            }
        } else if (!pattern) {
            pattern = tok;
        } else if (nfiles < 256) {
            files[nfiles++] = tok;
        }
        tok = strtok_r(NULL, " ", &saveptr);
    }

    if (!pattern || nfiles == 0)
        BOF_ERROR("Usage: grep [-inv] <pattern> <file> [file2 ...]");

    char pat_lower[1024];
    strncpy(pat_lower, pattern, sizeof(pat_lower) - 1);
    pat_lower[sizeof(pat_lower) - 1] = '\0';
    if (ignore_case) strtolower(pat_lower);

    for (int f = 0; f < nfiles; f++) {
        FILE *fp = fopen(files[f], "r");
        if (!fp) {
            BeaconPrintf(CALLBACK_ERROR, "grep: %s: %s\n", files[f], strerror(errno));
            continue;
        }

        char line[8192];
        int linenum = 0;
        while (fgets(line, sizeof(line), fp)) {
            linenum++;
            char *search_in = line;
            char lower_line[8192];
            if (ignore_case) {
                strncpy(lower_line, line, sizeof(lower_line) - 1);
                lower_line[sizeof(lower_line) - 1] = '\0';
                strtolower(lower_line);
                search_in = lower_line;
            }

            int match = (strstr(search_in, ignore_case ? pat_lower : pattern) != NULL);
            if (invert) match = !match;

            if (match) {
                /* Remove trailing newline for cleaner output */
                size_t len = strlen(line);
                if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';

                if (nfiles > 1 && show_num)
                    BeaconPrintf(CALLBACK_OUTPUT, "%s:%d:%s\n", files[f], linenum, line);
                else if (nfiles > 1)
                    BeaconPrintf(CALLBACK_OUTPUT, "%s:%s\n", files[f], line);
                else if (show_num)
                    BeaconPrintf(CALLBACK_OUTPUT, "%d:%s\n", linenum, line);
                else
                    BeaconPrintf(CALLBACK_OUTPUT, "%s\n", line);
            }
        }
        fclose(fp);
    }
}
