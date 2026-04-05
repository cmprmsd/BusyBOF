/*
 * mkdir.c — BOF: create directories
 * Usage: mkdir [-p] <dir> [dir2 ...]
 */
#include "bofdefs.h"

static int mkdir_p(const char *path, mode_t mode) {
    char tmp[PATH_MAX_BB];
    strncpy(tmp, path, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, mode) != 0 && errno != EEXIST) return -1;
            *p = '/';
        }
    }
    return mkdir(tmp, mode);
}

void go(char *args, int alen) {
    int parents = 0;

    /* Parse flags from args */
    if (args && alen > 0) {
        datap parser;
        BeaconDataParse(&parser, args, alen);
        char *argv_str = BeaconDataExtract(&parser, NULL);
        if (argv_str && *argv_str) {
            char *saveptr;
            char *tok = strtok_r(argv_str, " ", &saveptr);
            while (tok) {
                if (strcmp(tok, "-p") == 0) {
                    parents = 1;
                } else {
                    int ret = parents ? mkdir_p(tok, 0755) : mkdir(tok, 0755);
                    if (ret != 0 && !(parents && errno == EEXIST))
                        BeaconPrintf(CALLBACK_ERROR, "mkdir: '%s': %s\n", tok, strerror(errno));
                    else
                        BeaconPrintf(CALLBACK_OUTPUT, "Created: %s\n", tok);
                }
                tok = strtok_r(NULL, " ", &saveptr);
            }
        }
    }

    /* Pipe input: create each directory from previous stage */
    FILE *pipe = bof_pipe_input();
    if (pipe) {
        char line[PATH_MAX_BB];
        while (fgets(line, sizeof(line), pipe)) {
            line[strcspn(line, "\n")] = '\0';
            if (line[0]) {
                int ret = parents ? mkdir_p(line, 0755) : mkdir(line, 0755);
                if (ret != 0 && !(parents && errno == EEXIST))
                    BeaconPrintf(CALLBACK_ERROR, "mkdir: '%s': %s\n", line, strerror(errno));
                else
                    BeaconPrintf(CALLBACK_OUTPUT, "Created: %s\n", line);
            }
        }
        fclose(pipe);
    }
}
