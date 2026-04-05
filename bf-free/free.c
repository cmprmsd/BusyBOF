/*
 * free.c — BOF: display memory usage from /proc/meminfo
 * Usage: free
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) BOF_ERROR("free: cannot read /proc/meminfo");

    long mem_total = 0, mem_free = 0, mem_avail = 0, buffers = 0, cached = 0;
    long swap_total = 0, swap_free = 0;
    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "MemTotal: %ld", &mem_total) == 1) continue;
        if (sscanf(line, "MemFree: %ld", &mem_free) == 1) continue;
        if (sscanf(line, "MemAvailable: %ld", &mem_avail) == 1) continue;
        if (sscanf(line, "Buffers: %ld", &buffers) == 1) continue;
        if (sscanf(line, "Cached: %ld", &cached) == 1) continue;
        if (sscanf(line, "SwapTotal: %ld", &swap_total) == 1) continue;
        if (sscanf(line, "SwapFree: %ld", &swap_free) == 1) continue;
    }
    fclose(fp);

    long mem_used = mem_total - mem_free - buffers - cached;

    BeaconPrintf(CALLBACK_OUTPUT, "%-8s %12s %12s %12s %12s %12s\n",
        "", "total", "used", "free", "buff/cache", "available");
    BeaconPrintf(CALLBACK_OUTPUT, "%-8s %10ldM %10ldM %10ldM %10ldM %10ldM\n",
        "Mem:", mem_total/1024, mem_used/1024, mem_free/1024,
        (buffers+cached)/1024, mem_avail/1024);
    BeaconPrintf(CALLBACK_OUTPUT, "%-8s %10ldM %10ldM %10ldM\n",
        "Swap:", swap_total/1024, (swap_total-swap_free)/1024, swap_free/1024);
}
