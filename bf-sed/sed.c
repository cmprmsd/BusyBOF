/*
 * sed.c — BOF: simple stream editor (s/old/new/g only)
 * Usage: sed s/pattern/replacement/[g] <file>
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *expr = BeaconDataExtract(&parser, NULL);
    char *filepath = BeaconDataExtract(&parser, NULL);

    if (!expr || !*expr || !filepath || !*filepath)
        BOF_ERROR("Usage: sed 's/pattern/replacement/[g]' <file>");

    /* Parse s/old/new/g */
    if (expr[0] != 's' || expr[1] != '/')
        BOF_ERROR("sed: only s/old/new/[g] supported");

    char *p = expr + 2;
    char *delim2 = strchr(p, '/');
    if (!delim2) BOF_ERROR("sed: malformed expression");
    *delim2 = '\0';
    char *pattern = p;

    char *replacement = delim2 + 1;
    char *delim3 = strchr(replacement, '/');
    int global = 0;
    if (delim3) {
        *delim3 = '\0';
        if (strchr(delim3 + 1, 'g')) global = 1;
    }

    FILE *fp = fopen(filepath, "r");
    if (!fp) BOF_ERROR("sed: %s: %s", filepath, strerror(errno));

    size_t plen = strlen(pattern);
    size_t rlen = strlen(replacement);
    char line[8192];

    while (fgets(line, sizeof(line), fp)) {
        char out[16384];
        size_t oi = 0;
        size_t out_max = sizeof(out) - 1;
        char *cur = line;

        while (*cur) {
            char *found = strstr(cur, pattern);
            if (found) {
                /* Copy up to match */
                size_t pre = (size_t)(found - cur);
                if (oi + pre + rlen > out_max) { /* overflow guard */
                    size_t avail = out_max > oi ? out_max - oi : 0;
                    if (avail > 0) memcpy(out + oi, cur, avail < pre ? avail : pre);
                    oi = out_max;
                    break;
                }
                memcpy(out + oi, cur, pre); oi += pre;
                memcpy(out + oi, replacement, rlen); oi += rlen;
                cur = found + plen;
                if (!global) {
                    /* Copy rest verbatim */
                    size_t rest = strlen(cur);
                    if (oi + rest > out_max) rest = out_max - oi;
                    memcpy(out + oi, cur, rest); oi += rest;
                    break;
                }
            } else {
                size_t rest = strlen(cur);
                if (oi + rest > out_max) rest = out_max - oi;
                memcpy(out + oi, cur, rest); oi += rest;
                break;
            }
        }
        out[oi] = '\0';
        BeaconPrintf(CALLBACK_OUTPUT, "%s", out);
    }
    fclose(fp);
}
