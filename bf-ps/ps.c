/*
 * ps.c — BOF: list processes from /proc
 * Usage: ps [auxfwww]
 * Flags: a=all users, u=user format, x=no-tty too, f=forest, w=wide cmdline
 */
#include "bofdefs.h"

#define PS_MAX 1024

static void read_file_small(const char *path, char *buf, size_t bufsz) {
    buf[0] = '\0';
    FILE *fp = fopen(path, "r");
    if (!fp) return;
    size_t n = fread(buf, 1, bufsz - 1, fp);
    buf[n] = '\0';
    fclose(fp);
}

static long parse_field(const char *stat_line, int field_idx) {
    const char *p = strrchr(stat_line, ')');
    if (!p) return 0;
    p += 2;
    for (int i = 2; i < field_idx && *p; i++) {
        while (*p && *p != ' ') p++;
        while (*p == ' ') p++;
    }
    return atol(p);
}

/* Compact per-process data — ~360 bytes each, 1024 max = ~360KB */
struct ps_entry {
    int pid, ppid;
    int uid;
    long vsz_kb, rss_kb;
    long cputime;         /* utime + stime in ticks */
    char state;
    char user[24];
    char comm[64];
    char cmdline[256];
};

static struct ps_entry ps_buf[PS_MAX];
static int ps_count;

static void collect_procs(int wide) {
    ps_count = 0;
    DIR *procdir = opendir("/proc");
    if (!procdir) return;

    long page_kb = sysconf(_SC_PAGESIZE) / 1024;

    struct dirent *ent;
    while ((ent = readdir(procdir)) != NULL && ps_count < PS_MAX) {
        if (ent->d_name[0] < '0' || ent->d_name[0] > '9') continue;

        char path[280], statbuf[1024];
        snprintf(path, sizeof(path), "/proc/%s/stat", ent->d_name);
        read_file_small(path, statbuf, sizeof(statbuf));
        if (!statbuf[0]) continue;

        char *open = strchr(statbuf, '(');
        char *close = strrchr(statbuf, ')');
        if (!open || !close || close <= open) continue;

        struct ps_entry *e = &ps_buf[ps_count];
        e->pid = atoi(statbuf);

        size_t clen = (size_t)(close - open - 1);
        if (clen >= sizeof(e->comm)) clen = sizeof(e->comm) - 1;
        memcpy(e->comm, open + 1, clen);
        e->comm[clen] = '\0';

        e->state = *(close + 2);
        e->ppid     = (int)parse_field(statbuf, 3);
        e->cputime  = parse_field(statbuf, 13) + parse_field(statbuf, 14);
        e->vsz_kb   = parse_field(statbuf, 22) / 1024;
        e->rss_kb   = parse_field(statbuf, 23) * page_kb;

        /* UID from /proc/PID/status */
        snprintf(path, sizeof(path), "/proc/%s/status", ent->d_name);
        char statusbuf[512];
        read_file_small(path, statusbuf, sizeof(statusbuf));
        char *uidline = strstr(statusbuf, "\nUid:");
        if (!uidline) uidline = strstr(statusbuf, "Uid:");
        if (uidline) {
            if (*uidline == '\n') uidline++;
            sscanf(uidline + 4, "%d", &e->uid);
            struct passwd *pw = getpwuid((uid_t)e->uid);
            if (pw)
                strncpy(e->user, pw->pw_name, sizeof(e->user) - 1);
            else
                snprintf(e->user, sizeof(e->user), "%d", e->uid);
        } else {
            e->uid = -1;
            strcpy(e->user, "?");
        }

        /* Full cmdline (for -w) */
        e->cmdline[0] = '\0';
        if (wide) {
            snprintf(path, sizeof(path), "/proc/%s/cmdline", ent->d_name);
            FILE *fp = fopen(path, "r");
            if (fp) {
                size_t n = fread(e->cmdline, 1, sizeof(e->cmdline) - 1, fp);
                fclose(fp);
                for (size_t i = 0; i < n; i++)
                    if (e->cmdline[i] == '\0') e->cmdline[i] = ' ';
                while (n > 0 && e->cmdline[n - 1] == ' ') n--;
                e->cmdline[n] = '\0';
            }
        }

        ps_count++;
    }
    closedir(procdir);

    /* Insertion sort by PID */
    for (int i = 1; i < ps_count; i++) {
        struct ps_entry tmp = ps_buf[i];
        int j = i - 1;
        while (j >= 0 && ps_buf[j].pid > tmp.pid) {
            ps_buf[j + 1] = ps_buf[j];
            j--;
        }
        ps_buf[j + 1] = tmp;
    }
}

static void print_entry(struct ps_entry *e, int user_fmt, int wide,
                         long total_ticks, long memtotal, const char *prefix) {
    const char *cmd = (wide && e->cmdline[0]) ? e->cmdline : e->comm;

    if (user_fmt) {
        /* CPU% and MEM% as integer tenths to avoid float */
        int cpu10 = total_ticks > 0 ? (int)(e->cputime * 1000 / total_ticks) : 0;
        int mem10 = memtotal > 0 ? (int)(e->rss_kb * 1000 / memtotal) : 0;
        BeaconPrintf(CALLBACK_OUTPUT,
            "%-8s %7d %3d.%d %3d.%d %8ld %6ld %-6c %s%s\n",
            e->user, e->pid,
            cpu10 / 10, cpu10 % 10,
            mem10 / 10, mem10 % 10,
            e->vsz_kb, e->rss_kb,
            e->state, prefix, cmd);
    } else {
        BeaconPrintf(CALLBACK_OUTPUT,
            "%7d %7d %-6c %s%s\n",
            e->pid, e->ppid, e->state, prefix, cmd);
    }
}

static void print_forest(int parent, int depth, int user_fmt, int wide,
                          long total_ticks, long memtotal) {
    if (depth > 20) return; /* prevent runaway recursion */
    for (int i = 0; i < ps_count; i++) {
        if (ps_buf[i].ppid != parent) continue;

        char prefix[128] = "";
        for (int d = 0; d < depth - 1 && d < 30; d++)
            strcat(prefix, "  ");
        if (depth > 0)
            strcat(prefix, "\\_ ");

        print_entry(&ps_buf[i], user_fmt, wide, total_ticks, memtotal, prefix);
        print_forest(ps_buf[i].pid, depth + 1, user_fmt, wide, total_ticks, memtotal);
    }
}

void go(char *args, int alen) {
    int user_fmt = 0, forest = 0, wide = 0;

    if (args && alen > 0) {
        datap parser;
        BeaconDataParse(&parser, args, alen);
        char *argv_str = BeaconDataExtract(&parser, NULL);
        if (argv_str && *argv_str) {
            char *p = argv_str;
            if (*p == '-') p++;
            for (; *p; p++) {
                switch (*p) {
                case 'u': user_fmt = 1; break;
                case 'f': forest = 1; break;
                case 'w': wide = 1; break;
                }
            }
        }
    }

    collect_procs(wide);

    /* Uptime in ticks for CPU% */
    char ubuf[128];
    read_file_small("/proc/uptime", ubuf, sizeof(ubuf));
    long uptime_sec = atol(ubuf); /* integer part is fine */
    long total_ticks = uptime_sec * sysconf(_SC_CLK_TCK);

    /* Total memory for MEM% */
    char mbuf[256];
    read_file_small("/proc/meminfo", mbuf, sizeof(mbuf));
    long memtotal = 1;
    sscanf(mbuf, "MemTotal: %ld", &memtotal);
    if (memtotal <= 0) memtotal = 1;

    /* Header */
    if (user_fmt) {
        BeaconPrintf(CALLBACK_OUTPUT,
            "%-8s %7s %%CPU %%MEM %8s %6s %-6s %s\n",
            "USER", "PID", "VSZ", "RSS", "STAT", "COMMAND");
    } else {
        BeaconPrintf(CALLBACK_OUTPUT, "%7s %7s %-6s %s\n",
            "PID", "PPID", "STATE", "COMMAND");
    }

    if (forest) {
        print_forest(0, 0, user_fmt, wide, total_ticks, memtotal);
    } else {
        for (int i = 0; i < ps_count; i++)
            print_entry(&ps_buf[i], user_fmt, wide, total_ticks, memtotal, "");
    }
}
