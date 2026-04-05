/*
 * beacon.h - Beacon Object File API header
 * Based on the Outflank nix_bof_template
 * https://github.com/outflanknl/nix_bof_template
 */
#ifndef _BEACON_H_
#define _BEACON_H_

#include <stdint.h>
#include <stddef.h>

/* ── Data parser ────────────────────────────────────────────── */
typedef struct {
    char *original;
    char *buffer;
    int  length;
    int  size;
} datap;

void    BeaconDataParse(datap *parser, char *buffer, int size);
int     BeaconDataInt(datap *parser);
short   BeaconDataShort(datap *parser);
int     BeaconDataLength(datap *parser);
char   *BeaconDataExtract(datap *parser, int *size);

/* ── Output ─────────────────────────────────────────────────── */
typedef struct {
    char *original;
    char *buffer;
    int  length;
    int  size;
} formatp;

void    BeaconFormatAlloc(formatp *format, int maxsz);
void    BeaconFormatReset(formatp *format);
void    BeaconFormatFree(formatp *format);
void    BeaconFormatAppend(formatp *format, char *text, int len);
void    BeaconFormatPrintf(formatp *format, char *fmt, ...);
char   *BeaconFormatToString(formatp *format, int *size);
void    BeaconFormatInt(formatp *format, int value);

void    BeaconPrintf(int type, char *fmt, ...);
void    BeaconOutput(int type, char *data, int len);

/* Output types */
#define CALLBACK_OUTPUT      0x0
#define CALLBACK_OUTPUT_OEM  0x1e
#define CALLBACK_ERROR       0x0d
#define CALLBACK_OUTPUT_UTF8 0x20

/* ── Token / Process ────────────────────────────────────────── */
int     BeaconIsAdmin(void);
void    BeaconGetSpawnTo(int x86, char *buffer, int length);
int     BeaconSpawnTemporaryProcess(int x86, int ignoreToken, char **buffer, int *length);
void    BeaconCleanupProcess(int handle);

/* ── Utility ────────────────────────────────────────────────── */
int     toWideChar(char *src, wchar_t *dst, int max);

/* ── Dynamic Function Resolution (DFR) ──────────────────────── */
/* For Linux BOFs we use standard includes; DFR is Windows-only */

#endif /* _BEACON_H_ */
