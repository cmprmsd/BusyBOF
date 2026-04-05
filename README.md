# BusyBOF

Busybox-style Beacon Object Files for \*nix post-exploitation. Reimplements common Unix utilities as BOFs for use in stripped environments (Docker containers, Kubernetes pods, minimal VMs) where no binaries exist beyond the agent / implant.

Based on [nix_bof_template](https://github.com/outflanknl/nix_bof_template) by Outflank.

![](./busybof.png)

## Why BusyBOF?

Modern container environments ship zero utilities. No `ls`, no `cat`, no `grep`. **BusyBOF** gives operators a full Unix toolkit that runs in-memory through any ELF BOF loader — no binaries dropped to disk, no dependencies, no package managers.

```
[target]> bf-ls --args "-laF /etc"
drwxr-xr-x   1 root root  4096 Apr 05 08:12 /etc/
-rw-r--r--   1 root root   223 Apr 05 08:12 /etc/hosts
-rw-r--r--   1 root root    13 Apr 05 08:12 /etc/hostname
...

[target]> bf-ps
PID   PPID  USER     STATE  RSS     COMMAND
    1     0 root     S      5120    /agent
```

## Pipe Chaining

BOFs support Unix-style pipe chaining. Output from one BOF feeds into the next — all executed locally on the agent in a single command:

```
bf-env | bf-grep SHELL
bf-find --args "/etc -name *.conf" | bf-grep password
bf-tree --args "/var/log -F" | bf-du
bf-ls --args "-F /tmp" | bf-basename
bf-find --args "/app -name *.log" | bf-chmod --mode 644
bf-tree --args "/tmp -dF" | bf-rmdir
```

Pipes work with two categories of BOFs:

**Text processors** (grep, sort, head, tail, wc, awk, sed, cut, uniq, tr) receive the previous output as a file and process its contents — no code changes needed.

**Path processors** use `bof_pipe_input()` to detect pipe context via the `BOF_PIPE` environment variable and iterate over each line individually:

| BOF | Pipe behavior |
|-----|---------------|
| bf-basename | Output basename of each piped path |
| bf-dirname | Output dirname of each piped path |
| bf-du | Run du on each piped path |
| bf-stat | Stat each piped path |
| bf-chmod | Apply `--mode` to each piped path |
| bf-chown | Apply `--owner` to each piped path |
| bf-rm | Remove each piped path (respects `-rf` flags) |
| bf-rmdir | Remove each piped directory |
| bf-mkdir | Create each piped directory (respects `-p`) |
| bf-touch | Touch each piped path |
| bf-nl | Number each piped line |
| bf-rev | Reverse each piped line |

See [PIPE.md](PIPE.md) for the full integration guide if you want to add pipe support to your own BOFs or C2 framework.

## Build

```bash
make          # build all BOFs → build/
make bf-ls    # build single BOF
make list     # show available tools
make clean    # remove artifacts
```

Requires: `gcc` (or `x86_64-linux-gnu-gcc` for cross-compilation).

## Usage

Load any `.o` file from `build/` as a BOF in your C2 framework:

```
bof build/bf-ls/ls.o -la /tmp
bof build/bf-cat/cat.o /etc/passwd
bof build/bf-netstat/netstat.o -tlnp
bof build/bf-grep/grep.o -in "password" /etc/shadow
```

With Sliver style extension.json manifests and a controller that supports argument packing:

```
bf-ls --args "-laF /etc"
bf-grep --pattern password --file /etc/shadow
bf-awk --expression '{print $1}' --file /etc/passwd --delimiter :
bf-base64 --file /etc/hostname --mode decode
```

## Implemented Tools (64)

| Category | Tools |
|----------|-------|
| **File & Directory** | `cat` `cp` `diff` `find` `ln` `ls` `mkdir` `mv` `rm` `rmdir` `stat` `tar` `touch` `tree` |
| **Permissions** | `chmod` `chown` |
| **Text Processing** | `awk` `cut` `grep` `head` `nl` `paste` `rev` `sed` `sort` `tail` `tr` `uniq` `wc` |
| **System Info** | `date` `df` `dmesg` `du` `env` `free` `groups` `hostname` `id` `kill` `lsmod` `mount` `ps` `pwd` `sysctl` `uname` `uptime` `whoami` |
| **Network** | `arp` `ifconfig` `netstat` `nslookup` `ping` `route` `traceroute` `wget` |
| **Data & Encoding** | `base64` `md5sum` `od` `sha256sum` `strings` `xxd` |
| **Path** | `basename` `dirname` |
| **Misc** | `logger` |

## Extension Manifests

Each BOF includes an `extension.json` manifest for controller integration:

```json
{
  "name": "bf-grep",
  "commands": [{
    "command_name": "bf-grep",
    "help": "search files for pattern matches",
    "entrypoint": "go",
    "files": [{"os": "linux", "arch": "amd64", "path": "grep.o"}],
    "arguments": [
      {"name": "args", "type": "string", "desc": "[-inv] <pattern> <file>"}
    ]
  }]
}
```

Argument types: `string`, `int`, `short`, `file` (triggers local file completion), `wstring`. Arguments with `"choices"` trigger choice completion in the controller.

## Notes

- All hash implementations (MD5, SHA-256) are pure C with no external dependencies
- `sed` supports `s/old/new/[g]` substitution only
- `awk` supports `{print $N}` field extraction with configurable delimiter
- `wget` supports HTTP only (no TLS without external libs)
- `ping`/`traceroute` require CAP_NET_RAW or root
- `tree -F` outputs clean paths without tree art (suitable for piping)
- `arp` resolves hostnames via `/etc/hosts` (useful in Docker/K8s where Docker populates it)

## Further Development
This is a proof of concept for educational use only. Please refer to the [todo.md](todo.md) for additional tools that could be integrated into the collection.