/*
 * xxd.c — BOF: hex dump
 * Usage: xxd [-l len] <file>
 */
#include "bofdefs.h"
#include <ctype.h>

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: xxd [-l len] <file>");

    int limit = 0; /* 0 = no limit */
    char *filepath = NULL;
    char *saveptr;
    char *tok = strtok_r(argv_str, " ", &saveptr);

    while (tok) {
        if (strcmp(tok, "-l") == 0) {
            tok = strtok_r(NULL, " ", &saveptr);
            if (tok) limit = atoi(tok);
        } else {
            filepath = tok;
        }
        tok = strtok_r(NULL, " ", &saveptr);
    }

    if (!filepath) BOF_ERROR("Usage: xxd [-l len] <file>");

    FILE *fp = fopen(filepath, "rb");
    if (!fp) BOF_ERROR("xxd: %s: %s", filepath, strerror(errno));

    unsigned char buf[16];
    size_t offset = 0;
    size_t n;

    while ((n = fread(buf, 1, (limit > 0 && (size_t)limit - offset < 16) ? (size_t)limit - offset : 16, fp)) > 0) {
        if (limit > 0 && offset >= (size_t)limit) break;

        char line[128];
        size_t pos = (size_t)snprintf(line, sizeof(line), "%08zx: ", offset);

        for (size_t i = 0; i < 16; i++) {
            if (i < n) pos += (size_t)snprintf(line + pos, sizeof(line) - pos, "%02x", buf[i]);
            else pos += (size_t)snprintf(line + pos, sizeof(line) - pos, "  ");
            if (i % 2 == 1) pos += (size_t)snprintf(line + pos, sizeof(line) - pos, " ");
        }

        pos += (size_t)snprintf(line + pos, sizeof(line) - pos, " ");
        for (size_t i = 0; i < n; i++)
            line[pos++] = isprint(buf[i]) ? (char)buf[i] : '.';
        line[pos] = '\0';

        BeaconPrintf(CALLBACK_OUTPUT, "%s\n", line);
        offset += n;
    }
    fclose(fp);
}
