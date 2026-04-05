/*
 * logger.c — BOF: write message to syslog
 * Usage: logger [-t tag] [-p priority] <message>
 */
#include "bofdefs.h"
#include <sys/socket.h>
#include <sys/un.h>

void go(char *args, int alen) {
    if (!args || alen <= 0)
        BOF_ERROR("Usage: logger [-t tag] [-p priority] <message>");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);
    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: logger [-t tag] [-p priority] <message>");

    const char *tag = "user";
    int priority = 14; /* user.info = (1<<3)|6 = 14 */
    char *message = NULL;

    char *saveptr;
    char *tok = strtok_r(argv_str, " ", &saveptr);
    while (tok) {
        if (strcmp(tok, "-t") == 0) {
            tok = strtok_r(NULL, " ", &saveptr);
            if (tok) tag = tok;
        } else if (strcmp(tok, "-p") == 0) {
            tok = strtok_r(NULL, " ", &saveptr);
            if (tok) priority = atoi(tok);
        } else {
            /* Rest is the message — reconstruct from remaining tokens */
            message = tok;
            /* strtok already split, so remaining is accessible via saveptr */
            break;
        }
        tok = strtok_r(NULL, " ", &saveptr);
    }

    if (!message) BOF_ERROR("Usage: logger [-t tag] [-p priority] <message>");

    /* Reconstruct message (rejoin what strtok split) */
    char msg[4096];
    if (saveptr && *saveptr)
        snprintf(msg, sizeof(msg), "%s %s", message, saveptr);
    else
        snprintf(msg, sizeof(msg), "%s", message);

    /* Format syslog message: <priority>tag: message */
    char syslog_msg[4200];
    snprintf(syslog_msg, sizeof(syslog_msg), "<%d>%s: %s", priority, tag, msg);

    /* Send to /dev/log (Unix domain socket) */
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0) BOF_ERROR("logger: socket: %s", strerror(errno));

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "/dev/log", sizeof(addr.sun_path) - 1);

    if (sendto(fd, syslog_msg, strlen(syslog_msg), 0,
               (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        BOF_ERROR("logger: sendto /dev/log: %s", strerror(errno));
    }
    close(fd);
    BeaconPrintf(CALLBACK_OUTPUT, "Logged: %s\n", msg);
}
