/*
 * wget.c — BOF: simple HTTP GET download
 * Usage: wget <url> [output_file]
 * Supports http:// only (no TLS in a BOF without deps)
 */
#include "bofdefs.h"
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *url = BeaconDataExtract(&parser, NULL);
    char *outfile = BeaconDataExtract(&parser, NULL);

    if (!url || !*url)
        BOF_ERROR("Usage: wget <url> [output_file]");

    /* Treat empty optional output as NULL */
    if (outfile && !*outfile) outfile = NULL;

    /* Parse URL: http://host[:port]/path */
    if (strncmp(url, "http://", 7) != 0)
        BOF_ERROR("wget: only http:// supported (no TLS in BOF)");

    char host[256], path[2048];
    int port = 80;
    char *hp = url + 7;
    char *slash = strchr(hp, '/');
    if (slash) {
        size_t hlen = (size_t)(slash - hp);
        if (hlen >= sizeof(host)) hlen = sizeof(host) - 1;
        strncpy(host, hp, hlen);
        host[hlen] = '\0';
        strncpy(path, slash, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    } else {
        strncpy(host, hp, sizeof(host) - 1);
        host[sizeof(host) - 1] = '\0';
        strcpy(path, "/");
    }

    /* Check for port */
    char *colon = strchr(host, ':');
    if (colon) {
        *colon = '\0';
        port = atoi(colon + 1);
    }

    /* Derive output filename */
    char default_out[256] = "index.html";
    if (!outfile) {
        const char *base = strrchr(path, '/');
        if (base && base[1]) {
            strncpy(default_out, base + 1, sizeof(default_out) - 1);
            default_out[sizeof(default_out) - 1] = '\0';
        }
        outfile = default_out;
    }

    /* Resolve and connect */
    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", port);

    if (getaddrinfo(host, portstr, &hints, &res) != 0)
        BOF_ERROR("wget: cannot resolve '%s'", host);

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0 || connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        freeaddrinfo(res);
        BOF_ERROR("wget: connect to %s:%d failed: %s", host, port, strerror(errno));
    }
    freeaddrinfo(res);

    /* Send HTTP request */
    char req[4096];
    int rlen = snprintf(req, sizeof(req),
        "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: busybof/1.0\r\nConnection: close\r\n\r\n",
        path, host);
    send(sock, req, (size_t)rlen, 0);

    /* Read response */
    FILE *fp = fopen(outfile, "wb");
    if (!fp) { close(sock); BOF_ERROR("wget: cannot create '%s'", outfile); }

    char buf[8192];
    size_t total = 0;
    int header_done = 0;
    ssize_t n;
    while ((n = recv(sock, buf, sizeof(buf) - 1, 0)) > 0) {
        if (!header_done) {
            /* Null-terminate so strstr is safe */
            buf[n] = '\0';
            /* Find end of HTTP headers */
            char *end = strstr(buf, "\r\n\r\n");
            if (end) {
                header_done = 1;
                end += 4;
                size_t body_len = (size_t)(n - (end - buf));
                if (body_len > 0) {
                    fwrite(end, 1, body_len, fp);
                    total += body_len;
                }
                continue;
            }
        } else {
            fwrite(buf, 1, (size_t)n, fp);
            total += (size_t)n;
        }
    }

    fclose(fp);
    close(sock);
    BeaconPrintf(CALLBACK_OUTPUT, "'%s' saved [%zu bytes]\n", outfile, total);
}
