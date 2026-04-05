/*
 * wc.c — BOF: word, line, byte count
 * Usage: wc [-lwc] <file> [file2 ...]
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    if (!args || alen <= 0)
        BOF_ERROR("Usage: wc [-lwc] <file> [file2 ...]");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: wc [-lwc] <file> [file2 ...]");

    int show_lines = 0, show_words = 0, show_bytes = 0;
    int explicit_flags = 0;
    char *saveptr;
    char *tok = strtok_r(argv_str, " ", &saveptr);

    /* First pass: parse flags */
    char *files[256];
    int nfiles = 0;

    while (tok) {
        if (tok[0] == '-' && tok[1]) {
            explicit_flags = 1;
            for (int i = 1; tok[i]; i++) {
                if (tok[i] == 'l') show_lines = 1;
                if (tok[i] == 'w') show_words = 1;
                if (tok[i] == 'c') show_bytes = 1;
            }
        } else if (nfiles < 256) {
            files[nfiles++] = tok;
        }
        tok = strtok_r(NULL, " ", &saveptr);
    }

    if (!explicit_flags) { show_lines = show_words = show_bytes = 1; }

    long tl = 0, tw = 0, tb = 0;
    for (int f = 0; f < nfiles; f++) {
        FILE *fp = fopen(files[f], "r");
        if (!fp) {
            BeaconPrintf(CALLBACK_ERROR, "wc: %s: %s\n", files[f], strerror(errno));
            continue;
        }

        long lines = 0, words = 0, bytes = 0;
        int c, in_word = 0;
        while ((c = fgetc(fp)) != EOF) {
            bytes++;
            if (c == '\n') lines++;
            if (c == ' ' || c == '\t' || c == '\n') {
                in_word = 0;
            } else if (!in_word) {
                in_word = 1;
                words++;
            }
        }
        fclose(fp);

        char out[256] = "";
        if (show_lines) { char tmp[32]; snprintf(tmp, sizeof(tmp), "%7ld ", lines); strcat(out, tmp); }
        if (show_words) { char tmp[32]; snprintf(tmp, sizeof(tmp), "%7ld ", words); strcat(out, tmp); }
        if (show_bytes) { char tmp[32]; snprintf(tmp, sizeof(tmp), "%7ld ", bytes); strcat(out, tmp); }
        BeaconPrintf(CALLBACK_OUTPUT, "%s%s\n", out, files[f]);

        tl += lines; tw += words; tb += bytes;
    }

    if (nfiles > 1) {
        char out[256] = "";
        if (show_lines) { char tmp[32]; snprintf(tmp, sizeof(tmp), "%7ld ", tl); strcat(out, tmp); }
        if (show_words) { char tmp[32]; snprintf(tmp, sizeof(tmp), "%7ld ", tw); strcat(out, tmp); }
        if (show_bytes) { char tmp[32]; snprintf(tmp, sizeof(tmp), "%7ld ", tb); strcat(out, tmp); }
        BeaconPrintf(CALLBACK_OUTPUT, "%stotal\n", out);
    }
}
