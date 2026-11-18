#!/usr/bin/env bash
set -euo pipefail

binary=$1
key_file=$(mktemp /tmp/m3t4_integration.XXXXXX.key)
producer_log=$(mktemp /tmp/m3t4_producer.XXXXXX.log)
consumer_one_log=$(mktemp /tmp/m3t4_consumer1.XXXXXX.log)
consumer_two_log=$(mktemp /tmp/m3t4_consumer2.XXXXXX.log)

cleanup() {
  kill "${consumer_one_pid:-}" "${consumer_two_pid:-}" "${producer_pid:-}" \
    2>/dev/null || true
  wait 2>/dev/null || true
  rm -f "${key_file}" "${producer_log}" "${consumer_one_log}" \
    "${consumer_two_log}"
}
trap cleanup EXIT

"${binary}" -c "${key_file}" 0 >"${consumer_one_log}" 2>&1 && {
  echo "FAIL: consumer exited successfully without producer"
  exit 1
}

"${binary}" -p "${key_file}" 512 0 0 >"${producer_log}" 2>&1 &
producer_pid=$!

sleep 1

"${binary}" -c "${key_file}" 0 >"${consumer_one_log}" 2>&1 &
consumer_one_pid=$!
"${binary}" -c "${key_file}" 0 >"${consumer_two_log}" 2>&1 &
consumer_two_pid=$!

wait "${producer_pid}"
producer_pid=""

if ! timeout 30 tail --pid="${consumer_one_pid}" -f /dev/null; then
  echo "FAIL: consumer 1 did not finish"
  exit 1
fi
consumer_one_pid=""
wait "${consumer_two_pid}"
consumer_two_pid=""

grep -q "done: " "${producer_log}" || {
  echo "FAIL: producer did not report completion"
  cat "${producer_log}"
  exit 1
}

processed=$(cat "${consumer_one_log}" "${consumer_two_log}" | grep -c "min=")
[ "${processed}" -gt 0 ] || {
  echo "FAIL: consumers processed no blocks"
  cat "${consumer_one_log}" "${consumer_two_log}"
  exit 1
}

producer_blocks=$(sed -n 's/.*done: \([0-9]*\) blocks/\1/p' "${producer_log}")
[ "${processed}" -eq "${producer_blocks}" ] || {
  echo "FAIL: processed ${processed} blocks, producer created ${producer_blocks}"
  exit 1
}

echo "integration OK: ${processed} blocks processed by two consumers"
