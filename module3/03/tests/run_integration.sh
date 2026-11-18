#!/usr/bin/env bash
# Integration test for the p2p mq chat: an owner and two peers exchange
# messages, peer numbering follows the join order, a leaving peer does
# not disturb the rest, and the owner unlinks the queues on exit.
set -euo pipefail

BIN=${1:?usage: run_integration.sh <path to mq_chat>}
BASE="it_mq_chat_$$"
DIR=$(mktemp -d)
trap 'rm -rf "$DIR"; rm -f /dev/mqueue/"$BASE"_1 /dev/mqueue/"$BASE"_2 2>/dev/null || true' EXIT

# Owner: broadcasts a greeting, watches peers come and go, quits via Ctrl-D.
{
  sleep 2
  printf 'hello-all\n'
  sleep 3
} | timeout 20 "$BIN" "$BASE" >"$DIR/owner.out" 2>"$DIR/owner.err" &
OWNER_PID=$!

sleep 0.5

# Peer A: the first to join, so it must become peer1. Leaves at t=3.
{
  sleep 1
  printf 'from-a\n'
  sleep 2
} | timeout 20 "$BIN" "$BASE" >"$DIR/peer_a.out" 2>"$DIR/peer_a.err" &
PEER_A_PID=$!

sleep 0.5

# Peer B: joins second, so it must become peer2. Stays until the owner
# sends the quit announcement.
{
  sleep 1
  printf 'from-b\n'
  sleep 5
} | timeout 20 "$BIN" "$BASE" >"$DIR/peer_b.out" 2>"$DIR/peer_b.err" &
PEER_B_PID=$!

wait "$OWNER_PID"
wait "$PEER_A_PID"
wait "$PEER_B_PID"

# The owner sees both peers with their numbers.
grep -q "peer1> from-a" "$DIR/owner.out"
grep -q "peer2> from-b" "$DIR/owner.out"

# Both peers see the owner's broadcast.
grep -q "owner> hello-all" "$DIR/peer_a.out"
grep -q "owner> hello-all" "$DIR/peer_b.out"

# peer1's leave is reported by name and does not stop the owner.
grep -q "peer1 left the chat" "$DIR/owner.err"

# The remaining peer is notified when the owner leaves.
grep -q "Owner left the chat" "$DIR/peer_b.err"

if [ -e "/dev/mqueue/$BASE"_1 ] || [ -e "/dev/mqueue/$BASE"_2 ]; then
  echo "FAIL: queues were not unlinked" >&2
  exit 1
fi

echo "integration OK"
