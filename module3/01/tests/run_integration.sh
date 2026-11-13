#!/bin/bash
set -u

BINARY="$(realpath "${1:?Binary path required}")"
WORK_DIR="$(mktemp -d)"
trap 'rm -rf "$WORK_DIR"' EXIT

cd "$WORK_DIR" || exit 1

PASSED=0
FAILED=0

pass() {
  PASSED=$((PASSED + 1))
  echo "  PASS: $1"
}

fail() {
  FAILED=$((FAILED + 1))
  echo "  FAIL: $1" >&2
}

assert_file_exists() {
  if [ -f "$1" ]; then
    pass "$2"
  else
    fail "$2 (missing: $1)"
  fi
}

assert_files_equal() {
  if diff -q "$1" "$2" > /dev/null 2>&1; then
    pass "$3"
  else
    fail "$3 ($1 and $2 differ)"
  fi
}

assert_contains() {
  if grep -q "$2" "$1" 2>/dev/null; then
    pass "$3"
  else
    fail "$3 (pattern '$2' not found in $1)"
  fi
}

assert_exit_code() {
  if [ "$1" -eq "$2" ]; then
    pass "$3"
  else
    fail "$3 (expected $2, got $1)"
  fi
}

reset_work_dir() {
  rm -rf "$WORK_DIR"/*
}

prepare_inputs() {
  echo "hello world" > file1.txt
  echo "second file content" > file2.txt
  printf 'line one\nline two\n' > file3.txt
  dd if=/dev/urandom of=file4.bin bs=1024 count=16 2>/dev/null
}

echo "Building test fixtures..."
prepare_inputs

echo
echo "Test: single file, unnamed pipe"
reset_work_dir
prepare_inputs
"$BINARY" file1.txt > /dev/null 2> err.log
assert_exit_code $? 0 "exit code 0"
assert_file_exists file1.txt.copy "copy created"
assert_files_equal file1.txt file1.txt.copy "content matches"

echo
echo "Test: multiple text files, unnamed pipe"
reset_work_dir
prepare_inputs
"$BINARY" file1.txt file2.txt file3.txt > /dev/null 2> err.log
assert_exit_code $? 0 "exit code 0"
assert_file_exists file1.txt.copy "file1.txt.copy created"
assert_file_exists file2.txt.copy "file2.txt.copy created"
assert_file_exists file3.txt.copy "file3.txt.copy created"
assert_files_equal file1.txt file1.txt.copy "file1 content matches"
assert_files_equal file2.txt file2.txt.copy "file2 content matches"
assert_files_equal file3.txt file3.txt.copy "file3 content matches"

echo
echo "Test: binary file, unnamed pipe"
reset_work_dir
prepare_inputs
"$BINARY" file4.bin > /dev/null 2> err.log
assert_exit_code $? 0 "exit code 0"
assert_file_exists file4.bin.copy "copy created"
assert_files_equal file4.bin file4.bin.copy "binary content matches"

echo
echo "Test: empty file, unnamed pipe"
reset_work_dir
prepare_inputs
: > empty.txt
"$BINARY" empty.txt > /dev/null 2> err.log
assert_exit_code $? 0 "exit code 0"
assert_file_exists empty.txt.copy "copy created"
if [ -s empty.txt.copy ]; then
  fail "empty copy should have size 0"
else
  pass "empty copy has size 0"
fi

echo
echo "Test: single file, named pipe"
reset_work_dir
prepare_inputs
"$BINARY" -p "$WORK_DIR/chatfifo" file1.txt > /dev/null 2> err.log
assert_exit_code $? 0 "exit code 0"
assert_file_exists file1.txt.copy "copy created"
assert_files_equal file1.txt file1.txt.copy "content matches"
if [ -e chatfifo.p2c ] || [ -e chatfifo.c2p ]; then
  fail "FIFOs should be cleaned up"
else
  pass "FIFOs cleaned up"
fi

echo
echo "Test: multiple files, named pipe"
reset_work_dir
prepare_inputs
"$BINARY" -p "$WORK_DIR/chatfifo" file1.txt file4.bin > /dev/null 2> err.log
assert_exit_code $? 0 "exit code 0"
assert_file_exists file1.txt.copy "file1.txt.copy created"
assert_file_exists file4.bin.copy "file4.bin.copy created"
assert_files_equal file1.txt file1.txt.copy "text content matches"
assert_files_equal file4.bin file4.bin.copy "binary content matches"
if [ -e chatfifo.p2c ] || [ -e chatfifo.c2p ]; then
  fail "FIFOs should be cleaned up"
else
  pass "FIFOs cleaned up"
fi

echo
echo "Test: nonexistent file alongside existing files"
reset_work_dir
prepare_inputs
"$BINARY" missing.txt file1.txt > /dev/null 2> err.log
assert_exit_code $? 0 "exit code 0"
assert_file_exists file1.txt.copy "valid file still copied"
assert_files_equal file1.txt file1.txt.copy "valid file content matches"
assert_contains err.log "missing.txt" "diagnostic mentions missing file"
if [ -e missing.txt.copy ]; then
  fail "no copy should be created for missing file"
else
  pass "no copy created for missing file"
fi

echo
echo "Test: directory in file list"
reset_work_dir
prepare_inputs
mkdir subdir
"$BINARY" subdir file1.txt > /dev/null 2> err.log
assert_exit_code $? 0 "exit code 0"
assert_file_exists file1.txt.copy "valid file still copied"
assert_contains err.log "subdir" "diagnostic mentions directory"
assert_contains err.log "not a regular file" "diagnostic explains reason"

echo
echo "Test: no arguments"
reset_work_dir
"$BINARY" > /dev/null 2> err.log
assert_exit_code $? 1 "exit code 1"
assert_contains err.log "Usage" "usage message printed"

echo
echo "Test: -p without base name"
reset_work_dir
"$BINARY" -p > /dev/null 2> err.log
assert_exit_code $? 1 "exit code 1"
assert_contains err.log "Usage" "usage message printed"

echo
echo "Test: -p base but no files"
reset_work_dir
"$BINARY" -p "$WORK_DIR/somefifo" > /dev/null 2> err.log
assert_exit_code $? 1 "exit code 1"
assert_contains err.log "Usage" "usage message printed"

echo
echo "Test: large file, unnamed pipe"
reset_work_dir
prepare_inputs
dd if=/dev/urandom of=large.bin bs=1M count=4 2>/dev/null
"$BINARY" large.bin > /dev/null 2> err.log
assert_exit_code $? 0 "exit code 0"
assert_file_exists large.bin.copy "copy created"
assert_files_equal large.bin large.bin.copy "large binary content matches"

echo
echo "======================================"
echo "Passed: $PASSED, Failed: $FAILED"
echo "======================================"

if [ "$FAILED" -ne 0 ]; then
  exit 1
fi