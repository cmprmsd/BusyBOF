/*
 * rev.c — BOF: reverse lines of a file
 * Usage: rev <file>
 */
#include "bofdefs.h"

static void reverse_line(char *s) {
    size_t len = strlen(s);
    for (size_t i = 0; i < len / 2; i++) {
        char tmp = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = tmp;
    }
}

void go(char *args, int alen) {
    FILE *pipe = bof_pipe_input();
    if (pipe) {
        char line[8192];
        while (fgets(line, sizeof(line), pipe)) {
            line[strcspn(line, "\n")] = '\0';
            reverse_line(line);
            BeaconPrintf(CALLBACK_OUTPUT, "%s\n", line);
        }
        fclose(pipe);
        return;
    }

    if (!args || alen <= 0) BOF_ERROR("Usage: rev <file>");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *filepath = BeaconDataExtract(&parser, NULL);
    if (!filepath || !*filepath) BOF_ERROR("Usage: rev <file>");

    FILE *fp = fopen(filepath, "r");
    if (!fp) BOF_ERROR("rev: %s: %s", filepath, strerror(errno));

    char line[8192];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        reverse_line(line);
        BeaconPrintf(CALLBACK_OUTPUT, "%s\n", line);
    }
    fclose(fp);
}
