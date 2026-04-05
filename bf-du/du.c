/*
 * du.c — BOF: estimate disk usage
 * Usage: du [-s] [path]
 */
#include "bofdefs.h"

#define DU_MAX_DEPTH 64

static long long du_recurse(const char *path, int summary_only, int depth) {
    struct stat st;
    if (lstat(path, &st) < 0) return 0;

    long long total = st.st_blocks * 512;

    /* Only recurse into real directories, skip symlinks to dirs */
    if (S_ISDIR(st.st_mode) && !S_ISLNK(st.st_mode) && depth < DU_MAX_DEPTH) {
        DIR *dir = opendir(path);
        if (!dir) return total;

        struct dirent *ent;
        char child[PATH_MAX_BB];
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
            snprintf(child, sizeof(child), "%s/%s", path, ent->d_name);
            total += du_recurse(child, summary_only, depth + 1);
        }
        closedir(dir);

        if (!summary_only)
            BeaconPrintf(CALLBACK_OUTPUT, "%lldK\t%s\n", total / 1024, path);
    }
    return total;
}

void go(char *args, int alen) {
    /* Pipe input: du each path from previous stage */
    FILE *pipe = bof_pipe_input();
    if (pipe) {
        char line[PATH_MAX_BB];
        while (fgets(line, sizeof(line), pipe)) {
            line[strcspn(line, "\n")] = '\0';
            /* Strip trailing / from directory paths */
            size_t len = strlen(line);
            if (len > 1 && line[len - 1] == '/') line[len - 1] = '\0';
            if (line[0]) {
                long long total = du_recurse(line, 1, 0);
                BeaconPrintf(CALLBACK_OUTPUT, "%lldK\t%s\n", total / 1024, line);
            }
        }
        fclose(pipe);
        return;
    }

    char *path = ".";
    int summary = 0;

    if (args && alen > 0) {
        datap parser;
        BeaconDataParse(&parser, args, alen);
        char *argv_str = BeaconDataExtract(&parser, NULL);
        if (argv_str && *argv_str) {
            char *saveptr;
            char *tok = strtok_r(argv_str, " ", &saveptr);
            while (tok) {
                if (strcmp(tok, "-s") == 0) summary = 1;
                else path = tok;
                tok = strtok_r(NULL, " ", &saveptr);
            }
        }
    }

    long long total = du_recurse(path, summary, 0);
    if (summary)
        BeaconPrintf(CALLBACK_OUTPUT, "%lldK\t%s\n", total / 1024, path);
}
