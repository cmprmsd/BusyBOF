/*
 * id.c — BOF: print user/group IDs
 * Usage: id
 */
#include "bofdefs.h"

void go(char *args, int alen) {
    uid_t uid = getuid(), euid = geteuid();
    gid_t gid = getgid(), egid = getegid();

    struct passwd *pw = getpwuid(uid);
    struct group *gr = getgrgid(gid);

    BeaconPrintf(CALLBACK_OUTPUT, "uid=%d(%s) gid=%d(%s) euid=%d egid=%d\n",
        uid, pw ? pw->pw_name : "?",
        gid, gr ? gr->gr_name : "?",
        euid, egid);

    /* Supplementary groups */
    gid_t groups[64];
    int ngroups = getgroups(64, groups);
    if (ngroups > 0) {
        BeaconPrintf(CALLBACK_OUTPUT, "groups=");
        for (int i = 0; i < ngroups; i++) {
            struct group *sg = getgrgid(groups[i]);
            BeaconPrintf(CALLBACK_OUTPUT, "%d(%s)%s",
                groups[i], sg ? sg->gr_name : "?",
                i < ngroups - 1 ? "," : "\n");
        }
    }
}
