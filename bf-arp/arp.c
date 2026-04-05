/*
 * arp.c — BOF: show ARP table from /proc/net/arp with hostname resolution
 * Usage: arp
 */
#include "bofdefs.h"
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void go(char *args, int alen) {
    FILE *fp = fopen("/proc/net/arp", "r");
    if (!fp) BOF_ERROR("arp: cannot read /proc/net/arp");

    char line[512];
    /* Header line — append Hostname column */
    if (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        BeaconPrintf(CALLBACK_OUTPUT, "%-17s %-11s %-11s %-21s %-8s %-8s %s\n",
            "IP address", "HW type", "Flags", "HW address", "Mask", "Device", "Hostname");
    }

    while (fgets(line, sizeof(line), fp)) {
        char ip[64], hwtype[16], flags[16], hwaddr[32], mask[16], dev[32];
        if (sscanf(line, "%63s %15s %15s %31s %15s %31s",
                   ip, hwtype, flags, hwaddr, mask, dev) < 6)
            continue;

        /* Reverse-resolve hostname (same as arp -a: checks /etc/hosts then DNS) */
        char host[NI_MAXHOST];
        const char *hostname = "?";
        struct sockaddr_in sa = {0};
        sa.sin_family = AF_INET;
        if (inet_pton(AF_INET, ip, &sa.sin_addr) == 1) {
            if (getnameinfo((struct sockaddr *)&sa, sizeof(sa),
                            host, sizeof(host), NULL, 0, NI_NAMEREQD) == 0)
                hostname = host;
        }

        BeaconPrintf(CALLBACK_OUTPUT, "%-17s %-11s %-11s %-21s %-8s %-8s %s\n",
            ip, hwtype, flags, hwaddr, mask, dev, hostname);
    }
    fclose(fp);
}
