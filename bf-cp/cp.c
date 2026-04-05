/*
 * cp.c — BOF: copy files
 * Usage: cp <source> <dest>
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *src = BeaconDataExtract(&parser, NULL);
    char *dst = BeaconDataExtract(&parser, NULL);

    if (!src || !*src || !dst || !*dst)
        BOF_ERROR("Usage: cp <source> <dest>");

    FILE *in = fopen(src, "rb");
    if (!in) BOF_ERROR("cp: cannot open '%s': %s", src, strerror(errno));

    /* If dst is a directory, append source filename */
    struct stat st;
    char finaldst[PATH_MAX_BB];
    if (stat(dst, &st) == 0 && S_ISDIR(st.st_mode)) {
        const char *base = strrchr(src, '/');
        base = base ? base + 1 : src;
        snprintf(finaldst, sizeof(finaldst), "%s/%s", dst, base);
    } else {
        strncpy(finaldst, dst, sizeof(finaldst) - 1);
        finaldst[sizeof(finaldst) - 1] = '\0';
    }

    FILE *out = fopen(finaldst, "wb");
    if (!out) {
        fclose(in);
        BOF_ERROR("cp: cannot create '%s': %s", finaldst, strerror(errno));
    }

    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            fclose(in); fclose(out);
            BOF_ERROR("cp: write error: %s", strerror(errno));
        }
    }

    /* Preserve permissions */
    if (fstat(fileno(in), &st) == 0)
        fchmod(fileno(out), st.st_mode);

    fclose(in);
    fclose(out);
    BeaconPrintf(CALLBACK_OUTPUT, "'%s' -> '%s'\n", src, finaldst);
}
