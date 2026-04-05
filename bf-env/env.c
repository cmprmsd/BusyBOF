/*
 * env.c — BOF: print environment variables
 * Usage: env [VARNAME]
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    if (args && alen > 0) {
        datap parser;
        BeaconDataParse(&parser, args, alen);
        char *argv_str = BeaconDataExtract(&parser, NULL);
        if (argv_str && *argv_str) {
            /* Print specific variable */
            char *val = getenv(argv_str);
            if (val)
                BeaconPrintf(CALLBACK_OUTPUT, "%s=%s\n", argv_str, val);
            else
                BeaconPrintf(CALLBACK_ERROR, "env: '%s' not set\n", argv_str);
            return;
        }
    }

    /* Print all — read from /proc/self/environ since extern environ
     * is not resolved in the BOF loader context */
    FILE *fp = fopen("/proc/self/environ", "r");
    if (!fp) BOF_ERROR("env: cannot read /proc/self/environ: %s", strerror(errno));

    char buf[8192];
    size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);
    buf[n] = '\0';

    /* Entries are NUL-separated — sanitize ANSI escape sequences
     * to prevent terminal color leaking */
    for (size_t i = 0; i < n; ) {
        const char *entry = &buf[i];
        size_t elen = strlen(entry);

        /* Print char-by-char, skipping ESC [ ... m sequences */
        for (size_t j = 0; j < elen; j++) {
            if (entry[j] == '\033' && j + 1 < elen && entry[j+1] == '[') {
                /* Skip until 'm' or end */
                j += 2;
                while (j < elen && entry[j] != 'm') j++;
                continue;
            }
            BeaconPrintf(CALLBACK_OUTPUT, "%c", entry[j]);
        }
        BeaconPrintf(CALLBACK_OUTPUT, "\n");

        i += elen + 1;
    }
}
