#!/usr/bin/env bash
set -euo pipefail

BINARY="${1:?usage: run_integration.sh <path-to-udp_chat>}"
PORT=$(( (RANDOM % 20000) + 20000 ))
WORK_DIR="$(mktemp -d)"
trap 'rm -rf "$WORK_DIR"' EXIT

FIFO_A="$WORK_DIR/fifo_a"
FIFO_B="$WORK_DIR/fifo_b"
mkfifo "$FIFO_A" "$FIFO_B"

"$BINARY" "$PORT" alice <"$FIFO_A" >"$WORK_DIR/alice.log" 2>&1 &
PID_A=$!
exec 3>"$FIFO_A"

"$BINARY" "$PORT" bob <"$FIFO_B" >"$WORK_DIR/bob.log" 2>&1 &
PID_B=$!
exec 4>"$FIFO_B"

sleep 1
printf 'hello bob\n' >&3
sleep 1
printf 'hi alice\n' >&4
sleep 1
kill -INT "$PID_B"
sleep 1
kill -INT "$PID_A"
wait "$PID_A" "$PID_B" 2>/dev/null || true
exec 3>&- 4>&-

fail() {
  echo "FAIL: $1" >&2
  echo "--- alice.log ---" >&2
  cat "$WORK_DIR/alice.log" >&2
  echo "--- bob.log ---" >&2
  cat "$WORK_DIR/bob.log" >&2
  exit 1
}

grep -q "bob joined the chat" "$WORK_DIR/alice.log" || fail "alice did not see bob join"
grep -q "hello bob" "$WORK_DIR/bob.log" || fail "bob did not receive alice's message"
grep -q "hi alice" "$WORK_DIR/alice.log" || fail "alice did not receive bob's message"
grep -q "bob left the chat" "$WORK_DIR/alice.log" || fail "alice did not see bob leave"

echo "integration: OK"
