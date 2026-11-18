#include "view.h"

#include <stdio.h>
#include <unistd.h>

void view_usage(const char *program_name) {
  fprintf(stderr,
          "usage: %s -p <key_file> <capacity> [produce_delay] [poll_delay]\n"
          "       %s -c <key_file> [poll_delay]\n",
          program_name, program_name);
}

void view_error(const char *message) {
  fprintf(stderr, "error: %s\n", message);
}

void view_shared_memory_exists() {
  fprintf(stderr, "error: producer already running\n");
}

void view_shared_memory_unavailable() {
  fprintf(stderr, "error: shared memory unavailable, start producer first\n");
}

void view_producer_started(const size_t data_capacity) {
  printf("[producer %ld] started, capacity %zu bytes\n", (long)getpid(),
         data_capacity);
}

void view_producer_appended(const uint32_t block_number, const uint32_t count) {
  printf("[producer %ld] block #%u: %u values\n", (long)getpid(), block_number,
         count);
}

void view_producer_waiting() {
  printf("[producer %ld] memory full, waiting for consumers\n", (long)getpid());
}

void view_producer_finished(const uint32_t block_count) {
  printf("[producer %ld] done: %u blocks\n", (long)getpid(), block_count);
}

void view_consumer_started() {
  printf("[consumer %ld] started\n", (long)getpid());
}

void view_consumer_processed(const uint32_t block_offset, const int32_t minimum,
                             const int32_t maximum, const uint32_t count) {
  printf("[consumer %ld] block @%u: count=%u min=%d max=%d\n", (long)getpid(),
         block_offset, count, minimum, maximum);
}

void view_consumer_finished() {
  printf("[consumer %ld] done\n", (long)getpid());
}
