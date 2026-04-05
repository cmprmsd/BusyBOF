/*
 * traceroute.c — BOF: trace route to host via UDP probes
 * Usage: traceroute <host>
 * Requires CAP_NET_RAW or root for ICMP receive
 */
#include "bofdefs.h"
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);

    if (!argv_str || !*argv_str)
        BOF_ERROR("Usage: traceroute <host>");

    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_INET;
    if (getaddrinfo(argv_str, NULL, &hints, &res) != 0)
        BOF_ERROR("traceroute: cannot resolve '%s'", argv_str);

    struct sockaddr_in *dest = (struct sockaddr_in *)res->ai_addr;
    char dest_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &dest->sin_addr, dest_ip, sizeof(dest_ip));

    BeaconPrintf(CALLBACK_OUTPUT, "traceroute to %s (%s), 30 hops max\n", argv_str, dest_ip);

    int send_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (send_sock < 0 || recv_sock < 0) {
        freeaddrinfo(res);
        BOF_ERROR("traceroute: socket: %s", strerror(errno));
    }

    struct timeval tv = {.tv_sec = 2, .tv_usec = 0};
    setsockopt(recv_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    for (int ttl = 1; ttl <= 30; ttl++) {
        setsockopt(send_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        struct sockaddr_in target = *dest;
        target.sin_port = htons((uint16_t)(33434 + ttl));

        struct timeval t1, t2;
        gettimeofday(&t1, NULL);

        sendto(send_sock, "x", 1, 0, (struct sockaddr *)&target, sizeof(target));

        char buf[512];
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        ssize_t n = recvfrom(recv_sock, buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen);

        if (n > 0) {
            gettimeofday(&t2, NULL);
            long us = (t2.tv_sec - t1.tv_sec) * 1000000L + (t2.tv_usec - t1.tv_usec);
            long ms_whole = us / 1000;
            long ms_frac  = (us % 1000) / 100;  /* tenths */
            char hop_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &from.sin_addr, hop_ip, sizeof(hop_ip));

            /* Try reverse DNS */
            char hostname[256] = "";
            getnameinfo((struct sockaddr *)&from, sizeof(from),
                        hostname, sizeof(hostname), NULL, 0, 0);

            if (hostname[0] && strcmp(hostname, hop_ip) != 0)
                BeaconPrintf(CALLBACK_OUTPUT, "%2d  %s (%s)  %ld.%ld ms\n", ttl, hostname, hop_ip, ms_whole, ms_frac);
            else
                BeaconPrintf(CALLBACK_OUTPUT, "%2d  %s  %ld.%ld ms\n", ttl, hop_ip, ms_whole, ms_frac);

            if (from.sin_addr.s_addr == dest->sin_addr.s_addr) break;
        } else {
            BeaconPrintf(CALLBACK_OUTPUT, "%2d  *\n", ttl);
        }
    }

    close(send_sock);
    close(recv_sock);
    freeaddrinfo(res);
}
