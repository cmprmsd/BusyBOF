/*
 * tr.c — BOF: translate/delete characters from stdin-like input (file)
 * Usage: tr <set1> <set2> <file>  OR  tr -d <set1> <file>
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);
    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: tr [-d] <set1> [set2] <file>");

    int delete_mode = 0;
    char *set1 = NULL, *set2 = NULL, *filepath = NULL;
    char *saveptr;
    char *tok = strtok_r(argv_str, " ", &saveptr);

    while (tok) {
        if (strcmp(tok, "-d") == 0) delete_mode = 1;
        else if (!set1) set1 = tok;
        else if (!delete_mode && !set2) set2 = tok;
        else filepath = tok;
        tok = strtok_r(NULL, " ", &saveptr);
    }

    if (delete_mode && !filepath) { filepath = set2; set2 = NULL; }
    if (!set1 || !filepath) BOF_ERROR("Usage: tr [-d] <set1> [set2] <file>");

    FILE *fp = fopen(filepath, "r");
    if (!fp) BOF_ERROR("tr: %s: %s", filepath, strerror(errno));

    /* Build translation table */
    unsigned char map[256];
    for (int i = 0; i < 256; i++) map[i] = (unsigned char)i;

    size_t s1len = strlen(set1);
    size_t s2len = set2 ? strlen(set2) : 0;

    if (!delete_mode && set2) {
        for (size_t i = 0; i < s1len; i++) {
            unsigned char from = (unsigned char)set1[i];
            unsigned char to = (i < s2len) ? (unsigned char)set2[i] : (unsigned char)set2[s2len - 1];
            map[from] = to;
        }
    }

    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (delete_mode) {
            if (!memchr(set1, c, s1len))
                BeaconPrintf(CALLBACK_OUTPUT, "%c", c);
        } else {
            BeaconPrintf(CALLBACK_OUTPUT, "%c", map[(unsigned char)c]);
        }
    }
    fclose(fp);
}
