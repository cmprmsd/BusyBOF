# BOF Pipe Chaining — Integration Guide

This document explains how BusyBOF pipe chaining works and how to integrate it into any project that uses an ELF BOF loader (bof-launcher, goffloader, or similar).

## Overview

BusyBOFs support Unix-style pipe chaining where the output of one BOF feeds into the next:

```
bf-find --args "/etc -name *.conf" | bf-grep password | bf-head --args "-n 5"
bf-tree --args "/var/log -F" | bf-du
bf-ls --args "-F /tmp" | bf-chmod --args "644"
```

Pipes execute entirely on the agent — a single command is sent over the network, and the agent runs all stages locally with zero additional traffic.

## Architecture

### Three Layers

```
┌─────────────────────────────────────────────────────┐
│  Controller (operator side)                         │
│  - Detects | in user input                          │
│  - Resolves each BOF name to its binary + packed    │
│    args                                             │
│  - Sends one command: bof <b64> <args> | bof <b64>  │
└──────────────────────┬──────────────────────────────┘
                       │ single network message
┌──────────────────────▼──────────────────────────────┐
│  Executor (agent side)                              │
│  - splitPipe() on unquoted |                        │
│  - Runs stages sequentially                         │
│  - Writes previous output to temp file              │
│  - Sets BOF_PIPE env var                            │
│  - Appends temp file path to next stage's args      │
│  - Cleans up temp files after pipeline completes    │
└──────────────────────┬──────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────┐
│  BOF (C code)                                       │
│  - Text processors: get file path in args, fopen()  │
│  - Path processors: check bof_pipe_input(), iterate │
└─────────────────────────────────────────────────────┘
```

### Data Flow Example

```
bf-env | bf-grep SHELL
```

1. Controller resolves both BOFs to base64 blobs, packs `SHELL` as args for grep
2. Sends to agent: `bof <env-b64> | bof <grep-b64> <packed-SHELL-b64>`
3. Agent executor splits on `|`:
   - Stage 1: runs bf-env → output is all environment variables
   - Writes output to `/tmp/.bofpipe-XXXXX`
   - Sets `BOF_PIPE=/tmp/.bofpipe-XXXXX`
   - Stage 2 args become: `bof <grep-b64> <packed-"SHELL /tmp/.bofpipe-XXXXX"-b64>`
   - Runs bf-grep → reads file, matches lines containing SHELL
4. Final output returned to controller

## Integration into Your Project

### Step 1: Executor-Level Pipe Splitting

Add pipe detection to your command executor. Split on unquoted `|`, execute stages sequentially:

```go
func splitPipe(s string) []string {
    var stages []string
    var current strings.Builder
    inSingle, inDouble := false, false

    for i := 0; i < len(s); i++ {
        ch := s[i]
        switch {
        case ch == '\'' && !inDouble:
            inSingle = !inSingle
            current.WriteByte(ch)
        case ch == '"' && !inSingle:
            inDouble = !inDouble
            current.WriteByte(ch)
        case ch == '|' && !inSingle && !inDouble:
            stages = append(stages, current.String())
            current.Reset()
        default:
            current.WriteByte(ch)
        }
    }
    if current.Len() > 0 {
        stages = append(stages, current.String())
    }
    return stages
}
```

### Step 2: Pipeline Execution with Temp Files

For each stage after the first, write the previous output to a temp file and pass it to the next stage:

```go
func executePipeline(stages []string) (string, error) {
    var prevOutput string
    var tmpFiles []string
    defer func() {
        for _, f := range tmpFiles { os.Remove(f) }
    }()

    for i, stage := range stages {
        stage = strings.TrimSpace(stage)
        if i > 0 && prevOutput != "" {
            tmp, _ := os.CreateTemp("", ".bofpipe-*")
            tmp.WriteString(prevOutput)
            tmp.Close()
            tmpFiles = append(tmpFiles, tmp.Name())

            // Side-channel for path-processing BOFs
            os.Setenv("BOF_PIPE", tmp.Name())

            // Append file path to the command's argument string
            stage += " " + tmp.Name()
        }
        prevOutput, _ = executeSingle(stage)
    }
    os.Unsetenv("BOF_PIPE")
    return prevOutput, nil
}
```

### Step 3: Merging Pipe File into Packed Args

When a BOF already has pre-packed binary arguments from the controller, the pipe file path must be **concatenated into the first packed string entry** — not added as a separate entry. This ensures single-extract BOFs that tokenize by space see the file path naturally.

The packed arg format is:

```
[4-byte total_payload_length]
  [4-byte string1_length][string1_bytes (null-terminated)]
  [4-byte string2_length][string2_bytes (null-terminated)]
  ...
```

To merge a pipe file path into existing packed args:

1. Read the first string entry
2. Strip its null terminator
3. Append ` /tmp/.bofpipe-XXXXX` (space + path)
4. Re-add null terminator
5. Update length headers

```go
func appendPackedArgs(existing []byte, extras []string) ([]byte, error) {
    suffix := " " + strings.Join(extras, " ")

    if len(existing) < 8 {
        return packAsRawString(strings.Join(extras, " "))
    }

    totalLen := binary.LittleEndian.Uint32(existing[0:4])
    payload  := existing[4 : 4+totalLen]
    firstLen := int(binary.LittleEndian.Uint32(payload[0:4]))
    firstData := payload[4 : 4+firstLen]
    rest      := payload[4+firstLen:]

    // Strip null, append suffix, re-add null
    var newFirst []byte
    if firstData[firstLen-1] == 0 {
        newFirst = append(newFirst, firstData[:firstLen-1]...)
    }
    newFirst = append(newFirst, []byte(suffix)...)
    newFirst = append(newFirst, 0)

    // Rebuild packed buffer
    newTotal := 4 + len(newFirst) + len(rest)
    result := make([]byte, 4+newTotal)
    binary.LittleEndian.PutUint32(result[0:4], uint32(newTotal))
    binary.LittleEndian.PutUint32(result[4:8], uint32(len(newFirst)))
    copy(result[8:], newFirst)
    copy(result[8+len(newFirst):], rest)
    return result, nil
}
```

**Why concatenate instead of adding a separate entry?** BOFs call `BeaconDataExtract()` a fixed number of times. Single-extract BOFs (grep, sort, head, etc.) call it once and tokenize the result by space. If the pipe file were a separate entry, they'd never see it.

### Step 4: The BOF_PIPE Environment Variable

The temp file path is also passed via the `BOF_PIPE` environment variable. This is the **side-channel** for path-processing BOFs.

**Why two mechanisms?**

| BOF Type | How it receives pipe data | Example |
|----------|--------------------------|---------|
| **Text processor** | File path in args → `fopen()` | grep, sort, head, tail, wc, awk, sed, cut |
| **Path processor** | `BOF_PIPE` env var → iterate lines | basename, dirname, du, stat, chmod, rm |

Text processors read FROM a file (grep searches the file, sort sorts its lines). The pipe file path works directly as their file argument.

Path processors operate ON paths (basename strips directory, du measures size). They need to iterate over lines in the pipe file, processing each path individually. The `BOF_PIPE` env var lets them detect pipe context without ambiguity.

### Step 5: BOF-Side Integration

Add the helper to your common BOF header:

```c
/* bofdefs.h */
static inline FILE *bof_pipe_input(void) {
    const char *p = getenv("BOF_PIPE");
    if (p && *p) return fopen(p, "r");
    return NULL;
}
```

#### Text-Processing BOFs (No Changes Needed)

BOFs that read from a file argument work automatically. The pipe file path is concatenated into their args:

```c
// bf-grep: already tokenizes args, opens file — pipe works as-is
void go(char *args, int alen) {
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);
    // argv_str = "SHELL /tmp/.bofpipe-XXXXX"
    // tokenize → pattern="SHELL", file="/tmp/.bofpipe-XXXXX"
    // fopen(file) → search → works
}
```

#### Path-Processing BOFs

Add `bof_pipe_input()` at the top of `go()`. If it returns non-NULL, iterate over lines:

```c
void go(char *args, int alen) {
    FILE *pipe = bof_pipe_input();
    if (pipe) {
        char line[4096];
        while (fgets(line, sizeof(line), pipe)) {
            line[strcspn(line, "\n")] = '\0';
            if (line[0]) {
                // Process this path (basename, stat, chmod, etc.)
                char *base = basename(line);
                BeaconPrintf(CALLBACK_OUTPUT, "%s\n", base);
            }
        }
        fclose(pipe);
        return;
    }

    // Normal single-path execution follows...
}
```

#### Path-Processing BOFs with Flags

BOFs like chmod and chown accept flags/mode AND file targets. Parse flags from user args first, then use pipe input for targets:

```c
void go(char *args, int alen) {
    // Parse mode from packed args (user provides --mode 755)
    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *modestr = BeaconDataExtract(&parser, NULL);
    unsigned int mode;
    sscanf(modestr, "%o", &mode);

    // Pipe input: apply mode to each piped path
    FILE *pipe = bof_pipe_input();
    if (pipe) {
        char line[4096];
        while (fgets(line, sizeof(line), pipe)) {
            line[strcspn(line, "\n")] = '\0';
            if (line[0]) chmod(line, (mode_t)mode);
        }
        fclose(pipe);
        return;
    }
    // ... fallback to target from second arg
}
```

## Controller-Side: Resolving Pipe Stages

The controller must resolve BOF extension names to their binary blobs before sending:

```go
func executePipeline(stages []string) {
    var resolved []string
    for i, stage := range stages {
        cmd, args := parseInput(stage)
        if isExtension(cmd) {
            // Resolve BOF binary + pack args
            // For pipe sinks (i > 0): skip required arg validation
            remote := resolveExtension(cmd, args, i > 0)
            resolved = append(resolved, remote)
        } else {
            resolved = append(resolved, stage) // raw command
        }
    }
    // Send as single command: "bof <b64> <args> | bof <b64> <args>"
    send(peer, strings.Join(resolved, " | "))
}
```

**Pipe sinks skip required arg validation** because the executor supplies the missing file/path argument at runtime. Without this, `bf-find ... | bf-chmod --mode 755` would fail since chmod's `--target` arg is missing.

## Summary

| Component | Responsibility |
|-----------|---------------|
| Controller | Detect `\|` in input, resolve BOF names to blobs, skip required arg validation for pipe sinks, send single command |
| Executor | Split on `\|`, write temp files, set `BOF_PIPE` env var, append file path to args, clean up |
| BOF handler | Merge pipe file path into pre-packed args (first string concatenation) |
| Text BOFs | No changes — file path in args, `fopen()` reads it |
| Path BOFs | Call `bof_pipe_input()`, iterate lines |
