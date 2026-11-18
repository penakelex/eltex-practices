#define _POSIX_C_SOURCE 202405L

#include "consumer.h"

#include "block_list.h"
#include "view.h"

#include <unistd.h>

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

bool consumer_run(const IpcOps *ipc_ops, const char *base_name,
                  const unsigned int poll_delay) {
  if (ipc_ops == nullptr || base_name == nullptr) {
    return false;
  }

  Ipc ipc;
  ipc_initialize(&ipc, ipc_ops);

  if (!ipc_memory_open(&ipc, base_name)) {
    view_shared_memory_unavailable();
    return false;
  }

  if (!ipc_semaphore_open(&ipc, base_name)) {
    view_error("cannot open semaphore");
    ipc_memory_close(&ipc);
    return false;
  }

  view_consumer_started();

  for (;;) {
    void *data = ipc_memory_data(&ipc);
    const size_t capacity = ipc_memory_data_capacity(&ipc);

    if (!ipc_semaphore_wait(&ipc)) {
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

    const bool finished = !found && ipc_memory_is_producer_finished(&ipc) &&
                          block_list_all_processed(data, capacity);

    ipc_semaphore_post(&ipc);

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

  ipc_semaphore_close(&ipc);
  ipc_memory_close(&ipc);
  return true;
}
