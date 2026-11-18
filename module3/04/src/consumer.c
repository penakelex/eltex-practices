#define _POSIX_C_SOURCE 202405L

#include "consumer.h"

#include "block_list.h"
#include "semaphore.h"
#include "shared_memory.h"
#include "view.h"

#include <unistd.h>

constexpr int SEMAPHORE_PROJECT_ID = 'S';

static void scan_block(const void *data, const uint32_t offset,
                       int32_t *minimum, int32_t *maximum, uint32_t *count) {
  const BlockHeader *header = block_list_header_at_const(data, offset);
  const int32_t *values = block_list_values_at_const(data, offset);

  *count = header->count;
  *minimum = values[0];
  *maximum = values[0];

  for (uint32_t index = 1; index < header->count; ++index) {
    if (values[index] < *minimum) {
      *minimum = values[index];
    }

    if (values[index] > *maximum) {
      *maximum = values[index];
    }
  }
}

bool consumer_run(const char *key_path, const unsigned int poll_delay) {
  SharedMemory memory;

  if (!shared_memory_open(&memory, key_path, 0)) {
    view_shared_memory_unavailable();
    return false;
  }

  Semaphore semaphore;

  if (!semaphore_open(&semaphore, key_path, SEMAPHORE_PROJECT_ID)) {
    view_error("cannot open semaphore");
    shared_memory_close(&memory);
    return false;
  }

  view_consumer_started();

  for (;;) {
    void *data = shared_memory_data(&memory);
    const size_t capacity = shared_memory_data_capacity(&memory);

    if (!semaphore_wait(&semaphore)) {
      break;
    }

    uint32_t offset = 0;
    const auto found = block_list_find_unprocessed(data, capacity, &offset);

    int32_t minimum = 0;
    int32_t maximum = 0;
    uint32_t count = 0;

    if (found) {
      scan_block(data, offset, &minimum, &maximum, &count);
      block_list_mark_processed(data, offset);
    }

    const bool finished = !found &&
                          shared_memory_is_producer_finished(&memory) &&
                          block_list_all_processed(data, capacity);

    semaphore_post(&semaphore);

    if (found) {
      view_consumer_processed(offset, minimum, maximum, count);
    }

    if (finished) {
      break;
    }

    if (poll_delay > 0) {
      sleep(poll_delay);
    }
  }

  view_consumer_finished();

  shared_memory_close(&memory);
  return true;
}
