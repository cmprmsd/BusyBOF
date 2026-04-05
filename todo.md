# BusyBOF — Tool Tracker

Busybox-style BOFs for *nix post-exploitation.
Based on [nix_bof_template](https://github.com/outflanknl/nix_bof_template).

## Legend
- [x] Implemented
- [~] Partial implementation (subset of features)
- [ ] Not yet implemented

---

## File & Directory Operations
- [x] cat — concatenate and print files
- [x] ls — list directory contents (-l, -a, -F full paths)
- [x] cp — copy files (preserves permissions)
- [x] mv — move/rename files
- [x] rm — remove files/dirs (-rf), pipe support
- [x] mkdir — create directories (-p), pipe support
- [x] rmdir — remove empty directories, pipe support
- [x] chmod — change file permissions (octal), pipe support
- [x] chown — change ownership (user:group), pipe support
- [x] touch — create file / update timestamps, pipe support
- [x] ln — create hard/symbolic links (-s)
- [x] stat — display file status, pipe support
- [x] head — first N lines (-n)
- [x] tail — last N lines (-n)
- [x] find — recursive file search (-name, -type)
- [x] diff — line-by-line file comparison
- [x] tree — recursive directory tree (-a, -d, -F, -L depth)
- [x] tar — create, list, extract POSIX tar archives (-cf, -tf, -xf)

## Text Processing
- [x] grep — pattern search (-inv, substring)
- [~] sed — stream editor (s/old/new/g only)
- [~] awk — field extraction ({print $N}, configurable delimiter)
- [x] sort — sort lines (-r, -n)
- [x] uniq — deduplicate adjacent lines (-c)
- [x] tr — translate/delete characters
- [x] cut — extract fields (-d, -f)
- [x] paste — merge files side-by-side
- [x] wc — word/line/byte count (-lwc)
- [x] nl — number lines
- [x] rev — reverse lines

## System Information
- [x] id — print user/group IDs + supplementary groups
- [x] whoami — effective user name
- [x] uname — system info (-a)
- [x] hostname — print hostname
- [x] uptime — system uptime + load averages
- [x] ps — process list (from /proc)
- [x] kill — send signal to process (named + numeric)
- [x] pwd — current working directory
- [x] env — print/query environment variables
- [x] date — current date/time (custom format)
- [x] df — disk space usage (per mount)
- [x] du — directory disk usage (-s), pipe support
- [x] free — memory usage (from /proc/meminfo)
- [x] mount — list mount points (read-only)
- [x] dmesg — kernel ring buffer
- [x] lsmod — list loaded kernel modules
- [x] sysctl — read kernel parameters (-a, key lookup)
- [x] groups — print group memberships

## Network
- [x] ifconfig — network interfaces (from /sys + ioctl)
- [x] netstat — connections (from /proc/net, -tlnp)
- [x] arp — ARP table with hostname resolution
- [x] ping — ICMP echo (-c count)
- [x] traceroute — route tracing (UDP probes)
- [x] wget — HTTP GET download (http:// only)
- [x] nslookup — DNS lookup (A/AAAA)
- [x] route — show routing table

## Data & Encoding
- [x] strings — extract printable strings from binary (-n)
- [x] xxd — hex dump (-l)
- [x] base64 — encode/decode (--mode)
- [x] md5sum — MD5 hash (pure C, no openssl)
- [x] sha256sum — SHA-256 hash (pure C, no openssl)
- [x] od — octal dump (-A, -t, -N)

## Path
- [x] basename — strip directory from path, pipe support
- [x] dirname — strip filename from path, pipe support
- [x] logger — write message to syslog

---

## Not Yet Implemented (Candidates)

### File Operations
- [ ] dd — block-level copy (disk/partition operations)
- [ ] gzip / gunzip — compression (shrink exfil data)
- [ ] bzip2 / bunzip2 — compression alternative
- [ ] unzip — extract ZIP archives
- [ ] cpio — archive utility

### System
- [ ] taskset — CPU affinity

### Network
- [ ] ip — advanced network config (ip addr, ip route)

---

## Rejected (not feasible as BOF)

| Tool | Reason |
|------|--------|
| vi, less, more, top | Interactive / requires terminal |
| nc (netcat) | Needs persistent socket + bidirectional I/O |
| telnet | Interactive remote terminal |
| httpd | Long-running server, blocks agent |
| su, passwd, adduser/deluser | Interactive, requires PAM |
| nohup | BOFs can't fork child processes |
| chroot | Changes root for current process only; BOF exits immediately |
| nice/renice | Minimal operational value |
| expand/unexpand, fold | Trivial, no operational value |
| dpkg, rpm | Full package managers, wrapper binaries not BOFs |
| crontab | Interactive editor mode |
| ftpget/ftpput | FTP rarely available; C2 has file transfer |
| sleep | Blocks agent thread, harmful in C2 context |
| echo | Just prints args back; no operational value |
| xargs | BOFs can't fork; pipe chaining replaces this |
| tee | No stdin in BOF context; just a broken cat+cp |
| cal | No operational value |
