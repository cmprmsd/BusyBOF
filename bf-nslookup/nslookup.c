/*
 * nslookup.c — BOF: DNS lookup
 * Usage: nslookup <hostname>
 */
#include "bofdefs.h"
#include <netdb.h>
#include <arpa/inet.h>

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: nslookup <hostname>");

    struct addrinfo hints = {0}, *res, *p;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(argv_str, NULL, &hints, &res);
    if (err != 0)
        BOF_ERROR("nslookup: %s: %s", argv_str, gai_strerror(err));

    BeaconPrintf(CALLBACK_OUTPUT, "Name: %s\n", argv_str);
    for (p = res; p != NULL; p = p->ai_next) {
        char ip[INET6_ADDRSTRLEN];
        if (p->ai_family == AF_INET) {
            struct sockaddr_in *s = (struct sockaddr_in *)p->ai_addr;
            inet_ntop(AF_INET, &s->sin_addr, ip, sizeof(ip));
            BeaconPrintf(CALLBACK_OUTPUT, "Address: %s (IPv4)\n", ip);
        } else if (p->ai_family == AF_INET6) {
            struct sockaddr_in6 *s = (struct sockaddr_in6 *)p->ai_addr;
            inet_ntop(AF_INET6, &s->sin6_addr, ip, sizeof(ip));
            BeaconPrintf(CALLBACK_OUTPUT, "Address: %s (IPv6)\n", ip);
        }
    }
    freeaddrinfo(res);
}
