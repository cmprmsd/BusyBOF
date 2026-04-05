/*
 * chown.c — BOF: change file ownership
 * Usage: chown <user[:group]> <file> [file2 ...]
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    if (!args || alen <= 0)
        BOF_ERROR("Usage: chown <user[:group]> <file> [file2 ...]");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *owner = BeaconDataExtract(&parser, NULL);
    char *target = BeaconDataExtract(&parser, NULL);

    if (!owner || !*owner)
        BOF_ERROR("Usage: chown <user[:group]> <file> [file2 ...]");

    /* Parse user:group */
    uid_t uid = (uid_t)-1;
    gid_t gid = (gid_t)-1;
    char *colon = strchr(owner, ':');
    if (colon) {
        *colon = '\0';
        char *grpname = colon + 1;
        if (*grpname) {
            struct group *gr = getgrnam(grpname);
            if (gr) gid = gr->gr_gid;
            else gid = (gid_t)atoi(grpname);
        }
    }
    if (*owner) {
        struct passwd *pw = getpwnam(owner);
        if (pw) uid = pw->pw_uid;
        else uid = (uid_t)atoi(owner);
    }

    /* Pipe input: apply ownership to each path from previous stage */
    FILE *pipe = bof_pipe_input();
    if (pipe) {
        char line[PATH_MAX_BB];
        while (fgets(line, sizeof(line), pipe)) {
            line[strcspn(line, "\n")] = '\0';
            if (line[0]) {
                if (chown(line, uid, gid) != 0)
                    BeaconPrintf(CALLBACK_ERROR, "chown: '%s': %s\n", line, strerror(errno));
                else
                    BeaconPrintf(CALLBACK_OUTPUT, "'%s' -> %s\n", line, owner);
            }
        }
        fclose(pipe);
        return;
    }

    if (!target || !*target)
        BOF_ERROR("Usage: chown <user[:group]> <file> [file2 ...]");

    /* Target may contain multiple space-separated paths */
    char *saveptr;
    char *file = strtok_r(target, " ", &saveptr);
    while (file != NULL) {
        if (chown(file, uid, gid) != 0)
            BeaconPrintf(CALLBACK_ERROR, "chown: '%s': %s\n", file, strerror(errno));
        else
            BeaconPrintf(CALLBACK_OUTPUT, "'%s' -> %s\n", file, owner);
        file = strtok_r(NULL, " ", &saveptr);
    }
}
