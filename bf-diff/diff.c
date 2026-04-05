/*
 * diff.c — BOF: compare two files line-by-line
 * Usage: diff <file1> <file2>
 * Simple line-by-line comparison (not full LCS diff)
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *f1 = BeaconDataExtract(&parser, NULL);
    char *f2 = BeaconDataExtract(&parser, NULL);

    if (!f1 || !*f1 || !f2 || !*f2)
        BOF_ERROR("Usage: diff <file1> <file2>");

    FILE *fp1 = fopen(f1, "r");
    if (!fp1) BOF_ERROR("diff: %s: %s", f1, strerror(errno));
    FILE *fp2 = fopen(f2, "r");
    if (!fp2) { fclose(fp1); BOF_ERROR("diff: %s: %s", f2, strerror(errno)); }

    char l1[4096], l2[4096];
    int line = 0, diffs = 0;

    while (1) {
        char *r1 = fgets(l1, sizeof(l1), fp1);
        char *r2 = fgets(l2, sizeof(l2), fp2);
        line++;

        if (!r1 && !r2) break;

        if (!r1) {
            BeaconPrintf(CALLBACK_OUTPUT, "%da: %s", line, l2);
            diffs++;
        } else if (!r2) {
            BeaconPrintf(CALLBACK_OUTPUT, "%dd: %s", line, l1);
            diffs++;
        } else if (strcmp(l1, l2) != 0) {
            BeaconPrintf(CALLBACK_OUTPUT, "%dc:\n< %s> %s", line, l1, l2);
            diffs++;
        }
    }

    fclose(fp1);
    fclose(fp2);

    if (diffs == 0)
        BeaconPrintf(CALLBACK_OUTPUT, "Files are identical\n");
    else
        BeaconPrintf(CALLBACK_OUTPUT, "\n%d difference(s)\n", diffs);
}
