/*
 * base64.c — BOF: base64 encode/decode
 * Usage: base64 [-d] <file>
 */
#include "bofdefs.h"

static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int b64val(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static void encode(FILE *fp) {
    unsigned char in[3];
    char out[5];
    size_t n;

    while ((n = fread(in, 1, 3, fp)) > 0) {
        out[0] = b64chars[in[0] >> 2];
        out[1] = b64chars[((in[0] & 0x03) << 4) | (n > 1 ? (in[1] >> 4) : 0)];
        out[2] = n > 1 ? b64chars[((in[1] & 0x0F) << 2) | (n > 2 ? (in[2] >> 6) : 0)] : '=';
        out[3] = n > 2 ? b64chars[in[2] & 0x3F] : '=';
        out[4] = '\0';
        BeaconPrintf(CALLBACK_OUTPUT, "%s", out);
    }
    BeaconPrintf(CALLBACK_OUTPUT, "\n");
}

static void decode(FILE *fp) {
    char line[8192];
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        for (size_t i = 0; i + 3 < len;) {
            int a = b64val(line[i]), b = b64val(line[i+1]);
            int c = b64val(line[i+2]), d = b64val(line[i+3]);
            if (a < 0 || b < 0) { i++; continue; }

            unsigned char out[3];
            int n = 1;
            out[0] = (unsigned char)((a << 2) | (b >> 4));
            if (c >= 0) { out[1] = (unsigned char)(((b & 0xF) << 4) | (c >> 2)); n = 2; }
            if (d >= 0) { out[2] = (unsigned char)(((c & 0x3) << 6) | d); n = 3; }

            for (int j = 0; j < n; j++)
                BeaconPrintf(CALLBACK_OUTPUT, "%c", out[j]);
            i += 4;
        }
    }
}

void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *filepath = BeaconDataExtract(&parser, NULL);
    char *mode = BeaconDataExtract(&parser, NULL);

    if (!filepath || !*filepath)
        BOF_ERROR("Usage: base64 --file <file> [--mode encode|decode]");

    int do_decode = (mode && (strcmp(mode, "decode") == 0 || strcmp(mode, "-d") == 0 || strcmp(mode, "d") == 0));

    FILE *fp = fopen(filepath, "rb");
    if (!fp) BOF_ERROR("base64: %s: %s", filepath, strerror(errno));

    if (do_decode) decode(fp);
    else encode(fp);
    fclose(fp);
}
