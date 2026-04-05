/*
 * od.c — BOF: octal dump
 * Usage: od [-A x|o|d|n] [-t x1|o1|c] [-N bytes] <file>
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    if (!args || alen <= 0)
        BOF_ERROR("Usage: od [-A x|o|d] [-t x1|o1|c] [-N bytes] <file>");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);
    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: od [-A x|o|d] [-t x1|o1|c] [-N bytes] <file>");

    char addr_fmt = 'o';  /* o=octal, x=hex, d=decimal, n=none */
    char type = 'o';      /* o=octal bytes, x=hex bytes, c=chars */
    int max_bytes = 0;    /* 0 = all */
    char *filepath = NULL;

    char *saveptr;
    char *tok = strtok_r(argv_str, " ", &saveptr);
    while (tok) {
        if (strcmp(tok, "-A") == 0) {
            tok = strtok_r(NULL, " ", &saveptr);
            if (tok) addr_fmt = tok[0];
        } else if (strcmp(tok, "-t") == 0) {
            tok = strtok_r(NULL, " ", &saveptr);
            if (tok) type = tok[0];
        } else if (strcmp(tok, "-N") == 0) {
            tok = strtok_r(NULL, " ", &saveptr);
            if (tok) max_bytes = atoi(tok);
        } else {
            filepath = tok;
        }
        tok = strtok_r(NULL, " ", &saveptr);
    }

    if (!filepath) BOF_ERROR("Usage: od [-A x|o|d] [-t x1|o1|c] [-N bytes] <file>");

    FILE *fp = fopen(filepath, "rb");
    if (!fp) BOF_ERROR("od: %s: %s", filepath, strerror(errno));

    unsigned char buf[16];
    size_t offset = 0;
    size_t n;
    int total = 0;

    while ((n = fread(buf, 1, 16, fp)) > 0) {
        if (max_bytes > 0 && total + (int)n > max_bytes)
            n = (size_t)(max_bytes - total);

        /* Address */
        switch (addr_fmt) {
        case 'x': BeaconPrintf(CALLBACK_OUTPUT, "%07zx", offset); break;
        case 'd': BeaconPrintf(CALLBACK_OUTPUT, "%07zd", offset); break;
        case 'n': break;
        default:  BeaconPrintf(CALLBACK_OUTPUT, "%07zo", offset); break;
        }

        /* Data */
        for (size_t i = 0; i < n; i++) {
            switch (type) {
            case 'x':
                BeaconPrintf(CALLBACK_OUTPUT, " %02x", buf[i]);
                break;
            case 'c':
                if (buf[i] >= 0x20 && buf[i] < 0x7F)
                    BeaconPrintf(CALLBACK_OUTPUT, " %3c", buf[i]);
                else if (buf[i] == '\n') BeaconPrintf(CALLBACK_OUTPUT, "  \\n");
                else if (buf[i] == '\t') BeaconPrintf(CALLBACK_OUTPUT, "  \\t");
                else if (buf[i] == '\0') BeaconPrintf(CALLBACK_OUTPUT, "  \\0");
                else BeaconPrintf(CALLBACK_OUTPUT, " %03o", buf[i]);
                break;
            default:
                BeaconPrintf(CALLBACK_OUTPUT, " %03o", buf[i]);
                break;
            }
        }
        BeaconPrintf(CALLBACK_OUTPUT, "\n");

        offset += n;
        total += (int)n;
        if (max_bytes > 0 && total >= max_bytes) break;
    }

    /* Final address */
    if (addr_fmt != 'n') {
        switch (addr_fmt) {
        case 'x': BeaconPrintf(CALLBACK_OUTPUT, "%07zx\n", offset); break;
        case 'd': BeaconPrintf(CALLBACK_OUTPUT, "%07zd\n", offset); break;
        default:  BeaconPrintf(CALLBACK_OUTPUT, "%07zo\n", offset); break;
        }
    }
    fclose(fp);
}
