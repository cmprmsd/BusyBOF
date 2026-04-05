/*
 * rm.c — BOF: remove files/directories
 * Usage: rm [-rf] <path> [path2 ...]
 */
#include "bofdefs.h"

static int rm_recursive(const char *path) {
    struct stat st;
    if (lstat(path, &st) < 0) return -1;

    if (S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(path);
        if (!dir) return -1;

        struct dirent *ent;
        char child[PATH_MAX_BB];
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;
            snprintf(child, sizeof(child), "%s/%s", path, ent->d_name);
            rm_recursive(child);
        }
        closedir(dir);
        return rmdir(path);
    }
    return unlink(path);
}

static void rm_one(const char *path, int recursive, int force, int *count) {
    int ret = recursive ? rm_recursive(path) : unlink(path);
    if (ret != 0 && !force)
        BeaconPrintf(CALLBACK_ERROR, "rm: cannot remove '%s': %s\n", path, strerror(errno));
    else
        (*count)++;
}

void go(char *args, int alen) {
    int recursive = 0, force = 0, count = 0;

    /* Parse flags from args (needed even for pipe mode) */
    if (args && alen > 0) {
        datap parser;
        BeaconDataParse(&parser, args, alen);
        char *argv_str = BeaconDataExtract(&parser, NULL);
        if (argv_str && *argv_str) {
            char *saveptr;
            char *tok = strtok_r(argv_str, " ", &saveptr);
            while (tok) {
                if (tok[0] == '-') {
                    for (int i = 1; tok[i]; i++) {
                        if (tok[i] == 'r' || tok[i] == 'R') recursive = 1;
                        if (tok[i] == 'f') force = 1;
                    }
                } else {
                    rm_one(tok, recursive, force, &count);
                }
                tok = strtok_r(NULL, " ", &saveptr);
            }
        }
    }

    /* Pipe input: remove each path from previous stage */
    FILE *pipe = bof_pipe_input();
    if (pipe) {
        char line[PATH_MAX_BB];
        while (fgets(line, sizeof(line), pipe)) {
            line[strcspn(line, "\n")] = '\0';
            if (line[0]) rm_one(line, recursive, force, &count);
        }
        fclose(pipe);
    }

    if (count > 0)
        BeaconPrintf(CALLBACK_OUTPUT, "Removed %d item(s)\n", count);
    else if (!pipe)
        BOF_ERROR("Usage: rm [-rf] <path> [path2 ...]");
}
