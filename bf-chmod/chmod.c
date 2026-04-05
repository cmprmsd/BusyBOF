/*
 * chmod.c — BOF: change file permissions
 * Usage: chmod <mode> <file> [file2 ...]
 * Mode is octal (e.g. 755, 644)
 */
#include "bofdefs.h"

static void chmod_one(const char *file, mode_t mode) {
    if (chmod(file, mode) != 0)
        BeaconPrintf(CALLBACK_ERROR, "chmod: '%s': %s\n", file, strerror(errno));
    else
        BeaconPrintf(CALLBACK_OUTPUT, "'%s' -> %04o\n", file, (unsigned)mode);
}

void go(char *args, int alen) {
    if (!args || alen <= 0)
        BOF_ERROR("Usage: chmod <mode> <file> [file2 ...]");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *modestr = BeaconDataExtract(&parser, NULL);
    char *target = BeaconDataExtract(&parser, NULL);

    if (!modestr || !*modestr)
        BOF_ERROR("Usage: chmod <mode> <file> [file2 ...]");

    unsigned int mode;
    if (sscanf(modestr, "%o", &mode) != 1)
        BOF_ERROR("chmod: invalid mode '%s'", modestr);
    if (mode > 07777)
        BOF_ERROR("chmod: mode %o out of range (max 7777)", mode);

    /* Pipe input: apply mode to each path from previous stage */
    FILE *pipe = bof_pipe_input();
    if (pipe) {
        char line[PATH_MAX_BB];
        while (fgets(line, sizeof(line), pipe)) {
            line[strcspn(line, "\n")] = '\0';
            if (line[0]) chmod_one(line, (mode_t)mode);
        }
        fclose(pipe);
        return;
    }

    if (!target || !*target)
        BOF_ERROR("Usage: chmod <mode> <file> [file2 ...]");

    char *saveptr;
    char *file = strtok_r(target, " ", &saveptr);
    while (file != NULL) {
        chmod_one(file, (mode_t)mode);
        file = strtok_r(NULL, " ", &saveptr);
    }
}
