/*
 * tree.c — BOF: recursive directory tree listing
 * Usage: tree [path] [-a] [-d] [-F] [-L depth]
 * Flags: -a show hidden, -d dirs only, -F full paths, -L max depth
 */
#include "bofdefs.h"

typedef struct {
    int dirs;
    int files;
} counters_t;

static void print_tree(const char *path, const char *prefix, int is_last,
                       int depth, int max_depth, int show_all, int dirs_only,
                       int full_paths, counters_t *cnt) {
    if (max_depth > 0 && depth > max_depth)
        return;

    DIR *dir = opendir(path);
    if (!dir) return;

    /* Collect entries */
    char *names[4096];
    int is_dir[4096];
    int n = 0;
    struct dirent *ent;

    while ((ent = readdir(dir)) != NULL && n < 4096) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        if (!show_all && ent->d_name[0] == '.')
            continue;

        char fullpath[PATH_MAX_BB];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, ent->d_name);
        struct stat st;
        int isdir = 0;
        if (lstat(fullpath, &st) == 0)
            isdir = S_ISDIR(st.st_mode);

        if (dirs_only && !isdir)
            continue;

        names[n] = strdup(ent->d_name);
        if (!names[n]) break;
        is_dir[n] = isdir;
        n++;
    }
    closedir(dir);

    /* Sort alphabetically */
    /* Simple insertion sort to keep dirs and files sorted together */
    for (int i = 1; i < n; i++) {
        char *kn = names[i];
        int kd = is_dir[i];
        int j = i - 1;
        while (j >= 0 && strcmp(names[j], kn) > 0) {
            names[j + 1] = names[j];
            is_dir[j + 1] = is_dir[j];
            j--;
        }
        names[j + 1] = kn;
        is_dir[j + 1] = kd;
    }

    for (int i = 0; i < n; i++) {
        int last = (i == n - 1);
        char fullpath[PATH_MAX_BB];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, names[i]);

        if (full_paths) {
            /* Full-path mode: clean output (no tree art) for piping */
            if (is_dir[i]) {
                BeaconPrintf(CALLBACK_OUTPUT, "%s/\n", fullpath);
                cnt->dirs++;
                print_tree(fullpath, "", 0, depth + 1, max_depth,
                           show_all, dirs_only, full_paths, cnt);
            } else {
                BeaconPrintf(CALLBACK_OUTPUT, "%s\n", fullpath);
                cnt->files++;
            }
        } else {
            const char *connector = last ? "\xe2\x94\x94\xe2\x94\x80\xe2\x94\x80 "
                                         : "\xe2\x94\x9c\xe2\x94\x80\xe2\x94\x80 ";

            if (is_dir[i]) {
                BeaconPrintf(CALLBACK_OUTPUT, "%s%s%s/\n", prefix, connector, names[i]);
                cnt->dirs++;

                char child_prefix[PATH_MAX_BB];
                const char *ext = last ? "    " : "\xe2\x94\x82   ";
                snprintf(child_prefix, sizeof(child_prefix), "%s%s", prefix, ext);
                print_tree(fullpath, child_prefix, last, depth + 1, max_depth,
                           show_all, dirs_only, full_paths, cnt);
            } else {
                BeaconPrintf(CALLBACK_OUTPUT, "%s%s%s\n", prefix, connector, names[i]);
                cnt->files++;
            }
        }
        free(names[i]);
    }
}

void go(char *args, int alen) {
    char path[PATH_MAX_BB] = ".";
    int show_all = 0, dirs_only = 0, full_paths = 0, max_depth = 0;

    if (args && alen > 0) {
        datap parser;
        BeaconDataParse(&parser, args, alen);
        char *argv_str = BeaconDataExtract(&parser, NULL);
        if (argv_str && *argv_str) {
            char *saveptr;
            char *tok = strtok_r(argv_str, " ", &saveptr);
            while (tok) {
                if (strcmp(tok, "-a") == 0) {
                    show_all = 1;
                } else if (strcmp(tok, "-d") == 0) {
                    dirs_only = 1;
                } else if (strcmp(tok, "-F") == 0) {
                    full_paths = 1;
                } else if (strcmp(tok, "-L") == 0) {
                    tok = strtok_r(NULL, " ", &saveptr);
                    if (tok) max_depth = atoi(tok);
                } else if (tok[0] != '-') {
                    strncpy(path, tok, sizeof(path) - 1);
                    path[sizeof(path) - 1] = '\0';
                }
                tok = strtok_r(NULL, " ", &saveptr);
            }
        }
    }

    /* Verify path exists */
    struct stat st;
    if (stat(path, &st) < 0)
        BOF_ERROR("tree: %s: %s", path, strerror(errno));
    if (!S_ISDIR(st.st_mode))
        BOF_ERROR("tree: %s: Not a directory", path);

    BeaconPrintf(CALLBACK_OUTPUT, "%s\n", path);

    counters_t cnt = {0, 0};
    print_tree(path, "", 0, 1, max_depth, show_all, dirs_only, full_paths, &cnt);

    BeaconPrintf(CALLBACK_OUTPUT, "\n%d director%s, %d file%s\n",
        cnt.dirs, cnt.dirs == 1 ? "y" : "ies",
        cnt.files, cnt.files == 1 ? "" : "s");
}
