#!/bin/bash
# Comprehensive BOF reliability test
# Runs each BOF 10 times without args and 10 times with args
# Reports crashes (SIGSEGV, SIGABRT, etc.)

LOADER=~/boftest/main
BUILD=/data/Desktop/busybof/build
TD=/data/Desktop/busybof/testdata
ITERS=10

PASS=0
FAIL=0
SKIP=0
FAILURES=""

run_test() {
    local name="$1"
    local desc="$2"
    shift 2
    local bof="$BUILD/bf-$name/$name.o"

    if [ ! -f "$bof" ]; then
        echo "  SKIP  $name ($desc) - .o not found"
        ((SKIP++))
        return
    fi

    local crashes=0
    local errors=0
    for i in $(seq 1 $ITERS); do
        output=$("$LOADER" "$bof" "$@" 2>&1)
        rc=$?
        if echo "$output" | grep -qE 'SIGSEGV|SIGABRT|SIGBUS|segmentation|signal arrived|panic'; then
            ((crashes++))
        elif [ $rc -ne 0 ]; then
            ((errors++))
        fi
    done

    if [ $crashes -gt 0 ]; then
        echo "  CRASH $name ($desc) - $crashes/$ITERS crashed"
        ((FAIL++))
        FAILURES="$FAILURES\n  CRASH: $name ($desc) - $crashes/$ITERS"
    else
        echo "  OK    $name ($desc) - 0 crashes ($errors non-zero exits)"
        ((PASS++))
    fi
}

echo "========================================"
echo " BusyBOF Reliability Test Suite"
echo " $(date)"
echo " Iterations per test: $ITERS"
echo "========================================"
echo ""

# ── No-argument BOFs ──────────────────────────
echo "── Phase 1: No-argument tests (every BOF) ──"
echo ""

for dir in "$BUILD"/bf-*/; do
    name=$(basename "$dir" | sed 's/^bf-//')
    run_test "$name" "no-args"
done

echo ""
echo "── Phase 2: With-argument tests ──"
echo ""

# File readers
run_test cat    "read file"         "$TD/file1.txt"
run_test head   "default"           "$TD/lines.txt"
run_test head   "-n 2"              "-n" "2" "$TD/lines.txt"
run_test tail   "default"           "$TD/lines.txt"
run_test tail   "-n 2"              "-n" "2" "$TD/lines.txt"
run_test wc     "default"           "$TD/file1.txt"
run_test wc     "-l flag"           "-l" "$TD/lines.txt"
run_test xxd    "hex dump"          "$TD/binary.dat"
run_test xxd    "-l 16"             "-l" "16" "$TD/binary.dat"
run_test strings "default"          "$TD/binary.dat"
run_test strings "-n 3"             "-n" "3" "$TD/binary.dat"

# Hashing
run_test md5sum    "hash file"      "$TD/file1.txt"
run_test sha256sum "hash file"      "$TD/file1.txt"
run_test base64    "encode"         "$TD/file1.txt"
run_test base64    "decode -d"      "-d" "$TD/file1.txt"

# Text processing
run_test grep   "pattern"           "line" "$TD/lines.txt"
run_test grep   "-i flag"           "-i" "LINE" "$TD/lines.txt"
run_test grep   "-n flag"           "-n" "line" "$TD/lines.txt"
run_test grep   "-v flag"           "-v" "line1" "$TD/lines.txt"
run_test sort   "default"           "$TD/numbers.txt"
run_test sort   "-r reverse"        "-r" "$TD/numbers.txt"
run_test sort   "-n numeric"        "-n" "$TD/numbers.txt"
run_test uniq   "default"           "$TD/lines.txt"
run_test uniq   "-c count"          "-c" "$TD/lines.txt"
run_test cut    "fields"            "-d:" "-f1,2" "$TD/csv.txt"
run_test tr     "translate"         "a-z" "A-Z" "$TD/file1.txt"
run_test tr     "-d delete"         "-d" "aeiou" "$TD/file1.txt"
run_test sed    "substitute"        "s/hello/goodbye/" "$TD/file1.txt"
run_test paste  "two files"         "$TD/fruits.txt" "$TD/colors.txt"
run_test diff   "same files"        "$TD/file1.txt" "$TD/file1_copy.txt"
run_test diff   "diff files"        "$TD/fruits.txt" "$TD/colors.txt"
run_test awk    "print col"         "{print \$1}" "$TD/csv.txt"
run_test awk    "-F delim"          "-F:" "{print \$1,\$3}" "$TD/csv.txt"

# Path utilities
run_test basename "path"            "/usr/local/bin/test"
run_test basename "with suffix"     "/home/user/file.txt" ".txt"
run_test dirname  "path"            "/usr/local/bin/test"

# File system (read-only)
run_test ls     "current dir"       "$TD"
run_test ls     "-la flags"         "-la" "$TD"
run_test stat   "file"              "$TD/file1.txt"
run_test find   "by name"           "$TD" "-name" "*.txt"
run_test find   "by type"           "$TD" "-type" "d"
run_test du     "default"           "$TD"
run_test du     "-s summary"        "-s" "$TD"
run_test df     "root"              "/"

# File system (write - use temp paths)
TMPWRITE=/tmp/boftest_write_$$
mkdir -p "$TMPWRITE"
run_test touch  "create file"       "$TMPWRITE/newfile.txt"
run_test cp     "copy"              "$TD/file1.txt" "$TMPWRITE/copied.txt"
run_test mv     "rename"            "$TMPWRITE/copied.txt" "$TMPWRITE/renamed.txt"
run_test ln     "symlink"           "-s" "$TD/file1.txt" "$TMPWRITE/link.txt"
run_test mkdir  "create dir"        "$TMPWRITE/newdir"
run_test mkdir  "-p nested"         "-p" "$TMPWRITE/a/b/c"
run_test tee    "write"             "$TD/file1.txt" "$TMPWRITE/tee_out.txt"
run_test chmod  "644"               "644" "$TMPWRITE/newfile.txt"
# Clean up write tests between iterations
rm -rf "$TMPWRITE"

# System info
run_test echo   "text"              "hello" "world"
run_test echo   "-n no newline"     "-n" "hello"
run_test env    "HOME var"          "HOME"
run_test uname  "-a flag"           "-a"
run_test date   "format"            "+%Y-%m-%d"
run_test cal    "month year"        "3" "2026"

# Network (already tested netstat)
run_test netstat "no args"
run_test netstat "-tl flags"        "-tl"
run_test netstat "-u flag"          "-u"
run_test nslookup "resolve"         "localhost"

# Skipping these intentionally:
# kill   - would kill a process
# rm/rmdir - destructive
# ping   - needs raw socket / may timeout
# traceroute - needs raw socket / may timeout
# wget   - needs network, slow
# sleep  - too slow (waits N seconds)
# chown  - needs root

echo ""
echo "========================================"
echo " RESULTS"
echo "========================================"
echo " PASS:  $PASS"
echo " FAIL:  $FAIL"
echo " SKIP:  $SKIP"
echo " TOTAL: $((PASS + FAIL + SKIP))"
if [ -n "$FAILURES" ]; then
    echo ""
    echo " FAILURES:"
    echo -e "$FAILURES"
fi
echo "========================================"
