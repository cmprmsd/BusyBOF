/*
 * cut.c — BOF: extract fields/columns
 * Usage: cut -d<delim> -f<fields> <file>  OR  cut -c<range> <file>
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *spec = BeaconDataExtract(&parser, NULL);
    char *filepath = BeaconDataExtract(&parser, NULL);
    if (!spec || !*spec || !filepath || !*filepath)
        BOF_ERROR("Usage: cut -d<delim> -f<fields> <file>");

    char delim = '\t';
    int field = 0;
    char *saveptr;
    char *tok = strtok_r(spec, " ", &saveptr);

    while (tok) {
        if (strncmp(tok, "-d", 2) == 0) delim = tok[2] ? tok[2] : '\t';
        else if (strncmp(tok, "-f", 2) == 0) field = atoi(tok + 2);
        tok = strtok_r(NULL, " ", &saveptr);
    }

    if (field < 1)
        BOF_ERROR("Usage: cut -d<delim> -f<N> <file>");

    FILE *fp = fopen(filepath, "r");
    if (!fp) BOF_ERROR("cut: %s: %s", filepath, strerror(errno));

    char line[8192];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        char *cur = line;
        int f = 1;
        char *start = cur;

        while (*cur) {
            if (*cur == delim) {
                if (f == field) { *cur = '\0'; break; }
                f++;
                start = cur + 1;
            }
            cur++;
        }

        if (f >= field)
            BeaconPrintf(CALLBACK_OUTPUT, "%s\n", start);
    }
    fclose(fp);
}
