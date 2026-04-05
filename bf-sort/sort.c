/*
 * sort.c — BOF: sort lines of a file
 * Usage: sort [-r] [-n] <file>
 */
#include "bofdefs.h"

static int sort_reverse = 0;
static int sort_numeric = 0;

static int cmpfn(const void *a, const void *b) {
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    int r;
    if (sort_numeric) {
        double da = atof(sa), db = atof(sb);
        r = (da > db) - (da < db);
    } else {
        r = strcmp(sa, sb);
    }
    return sort_reverse ? -r : r;
}

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);
    if (!argv_str || !*argv_str) BOF_ERROR("Usage: sort [-rn] <file>");

    sort_reverse = 0; sort_numeric = 0;
    char *filepath = NULL;
    char *saveptr;
    char *tok = strtok_r(argv_str, " ", &saveptr);
    while (tok) {
        if (tok[0] == '-') {
            for (int i = 1; tok[i]; i++) {
                if (tok[i] == 'r') sort_reverse = 1;
                if (tok[i] == 'n') sort_numeric = 1;
            }
        } else filepath = tok;
        tok = strtok_r(NULL, " ", &saveptr);
    }
    if (!filepath) BOF_ERROR("Usage: sort [-rn] <file>");

    FILE *fp = fopen(filepath, "r");
    if (!fp) BOF_ERROR("sort: %s: %s", filepath, strerror(errno));

    char **lines = NULL;
    size_t nlines = 0, cap = 0;
    char buf[8192];
    while (fgets(buf, sizeof(buf), fp)) {
        if (nlines >= cap) {
            cap = cap ? cap * 2 : 256;
            char **tmp = realloc(lines, cap * sizeof(char *));
            if (!tmp) { BeaconPrintf(CALLBACK_ERROR, "sort: out of memory\n"); break; }
            lines = tmp;
        }
        char *dup = strdup(buf);
        if (!dup) { BeaconPrintf(CALLBACK_ERROR, "sort: out of memory\n"); break; }
        lines[nlines++] = dup;
    }
    fclose(fp);

    qsort(lines, nlines, sizeof(char *), cmpfn);
    for (size_t i = 0; i < nlines; i++) {
        BeaconPrintf(CALLBACK_OUTPUT, "%s", lines[i]);
        free(lines[i]);
    }
    free(lines);
}
