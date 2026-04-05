/*
 * touch.c — BOF: create file or update timestamps
 * Usage: touch <file> [file2 ...]
 */
#include "bofdefs.h"
#include <utime.h>

static void touch_one(const char *file) {
    if (utime(file, NULL) != 0) {
        int fd = open(file, O_WRONLY | O_CREAT, 0644);
        if (fd < 0)
            BeaconPrintf(CALLBACK_ERROR, "touch: '%s': %s\n", file, strerror(errno));
        else {
            close(fd);
            BeaconPrintf(CALLBACK_OUTPUT, "Created: %s\n", file);
        }
    }
}

void go(char *args, int alen) {
    FILE *pipe = bof_pipe_input();
    if (pipe) {
        char line[PATH_MAX_BB];
        while (fgets(line, sizeof(line), pipe)) {
            line[strcspn(line, "\n")] = '\0';
            if (line[0]) touch_one(line);
        }
        fclose(pipe);
        return;
    }

    if (!args || alen <= 0)
        BOF_ERROR("Usage: touch <file> [file2 ...]");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: touch <file> [file2 ...]");

    char *saveptr;
    char *file = strtok_r(argv_str, " ", &saveptr);
    while (file) {
        touch_one(file);
        file = strtok_r(NULL, " ", &saveptr);
    }
}
