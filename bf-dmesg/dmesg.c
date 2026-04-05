/*
 * dmesg.c — BOF: print kernel ring buffer
 * Usage: dmesg [-n lines]
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    int max_lines = 0; /* 0 = all */

    if (args && alen > 0) {
        datap parser;
        BeaconDataParse(&parser, args, alen);
        char *argv_str = BeaconDataExtract(&parser, NULL);
        if (argv_str && *argv_str) {
            char *saveptr;
            char *tok = strtok_r(argv_str, " ", &saveptr);
            while (tok) {
                if (strcmp(tok, "-n") == 0) {
                    tok = strtok_r(NULL, " ", &saveptr);
                    if (tok) max_lines = atoi(tok);
                }
                tok = strtok_r(NULL, " ", &saveptr);
            }
        }
    }

    /* Try /dev/kmsg first (modern), fall back to /proc/kmsg */
    FILE *fp = fopen("/dev/kmsg", "r");
    if (!fp) fp = fopen("/proc/kmsg", "r");
    if (!fp) {
        /* Last resort: use klogctl syscall via /proc */
        fp = fopen("/var/log/kern.log", "r");
        if (!fp) BOF_ERROR("dmesg: cannot access kernel log (need root or /var/log/kern.log)");
    }

    char line[1024];
    /* /dev/kmsg is non-blocking seekable; read from current position */
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        /* /dev/kmsg format: "priority,seq,timestamp,-;message" */
        char *msg = strchr(line, ';');
        if (msg) {
            msg++; /* skip ';' */
            BeaconPrintf(CALLBACK_OUTPUT, "%s", msg);
        } else {
            /* /proc/kmsg or kern.log: raw lines */
            BeaconPrintf(CALLBACK_OUTPUT, "%s", line);
        }
        count++;
        if (max_lines > 0 && count >= max_lines) break;
    }
    fclose(fp);
}
