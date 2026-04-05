/*
 * kill.c — BOF: send signal to process
 * Usage: kill [-signal] <pid> [pid2 ...]
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    if (!args || alen <= 0)
        BOF_ERROR("Usage: kill [-signal] <pid> [pid2 ...]");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: kill [-signal] <pid> [pid2 ...]");

    int sig = SIGTERM;
    char *saveptr;
    char *tok = strtok_r(argv_str, " ", &saveptr);

    while (tok) {
        if (tok[0] == '-') {
            sig = atoi(tok + 1);
            if (sig == 0 && strcmp(tok + 1, "0") != 0) {
                /* Named signal */
                if (strcasecmp(tok + 1, "HUP") == 0)  sig = SIGHUP;
                else if (strcasecmp(tok + 1, "INT") == 0)  sig = SIGINT;
                else if (strcasecmp(tok + 1, "KILL") == 0) sig = SIGKILL;
                else if (strcasecmp(tok + 1, "TERM") == 0) sig = SIGTERM;
                else if (strcasecmp(tok + 1, "STOP") == 0) sig = SIGSTOP;
                else if (strcasecmp(tok + 1, "CONT") == 0) sig = SIGCONT;
                else if (strcasecmp(tok + 1, "USR1") == 0) sig = SIGUSR1;
                else if (strcasecmp(tok + 1, "USR2") == 0) sig = SIGUSR2;
                else BOF_ERROR("kill: unknown signal '%s'", tok + 1);
            }
        } else {
            pid_t pid = (pid_t)atoi(tok);
            if (kill(pid, sig) != 0)
                BeaconPrintf(CALLBACK_ERROR, "kill: pid %d: %s\n", pid, strerror(errno));
            else
                BeaconPrintf(CALLBACK_OUTPUT, "Sent signal %d to pid %d\n", sig, pid);
        }
        tok = strtok_r(NULL, " ", &saveptr);
    }
}
