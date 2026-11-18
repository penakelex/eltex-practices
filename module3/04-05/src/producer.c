#define _POSIX_C_SOURCE 202405L

#include "producer.h"

#include "block_list.h"
#include "view.h"

#include <stdlib.h>
#include <time.h>
#include <unistd.h>

constexpr uint32_t PRODUCER_BLOCK_MAX_VALUES = 64;

static uint32_t random_block_size() {
  return (uint32_t)rand() % PRODUCER_BLOCK_MAX_VALUES + 1;
}

static void fill_random_values(int32_t *values, uint32_t count) {
  for (uint32_t index = 0; index < count; ++index) {
    values[index] = rand();
  }
}

bool producer_run(const IpcOps *ipc_ops, const char *base_name,
                  const size_t data_capacity, const unsigned int produce_delay,
                  const unsigned int poll_delay) {
  if (ipc_ops == nullptr || base_name == nullptr) {
    return false;
  }

  Ipc ipc;
  ipc_initialize(&ipc, ipc_ops);

  bool created = false;

  if (!ipc_memory_create(&ipc, base_name, data_capacity, &created)) {
    view_error("cannot create shared memory");
    return false;
  }

  if (!created) {
    view_shared_memory_exists();
    ipc_memory_close(&ipc);
    return false;
  }

  if (!ipc_semaphore_create(&ipc, base_name, &created) || !created) {
    view_error("cannot create semaphore");
    ipc_memory_close(&ipc);
    ipc_memory_remove(ipc_ops, base_name);
    return false;
  }

  if (!block_list_initialize(ipc_memory_data(&ipc),
                             ipc_memory_data_capacity(&ipc))) {
    view_error("cannot initialize block list");
    ipc_semaphore_remove(ipc_ops, base_name);
    ipc_memory_close(&ipc);
    ipc_memory_remove(ipc_ops, base_name);
    return false;
  }

  view_producer_started(data_capacity);
  srand((unsigned int)time(nullptr) ^ (unsigned int)getpid());

  uint32_t block_count = 0;
  int32_t values[PRODUCER_BLOCK_MAX_VALUES];
  bool waiting_reported = false;

  for (;;) {
    const uint32_t count = random_block_size();
    fill_random_values(values, count);

    void *data = ipc_memory_data(&ipc);
    const size_t capacity = ipc_memory_data_capacity(&ipc);

    ipc_semaphore_wait(&ipc);

    uint32_t offset = 0;
    const auto appended =
        block_list_append(data, capacity, values, count, &offset);
    bool finished = false;

    if (appended) {
      block_count += 1;
    } else if (block_list_all_processed(data, capacity)) {
      ipc_memory_set_producer_finished(&ipc);
      finished = true;
    }

    ipc_semaphore_post(&ipc);

    if (finished) {
      break;
    }

    if (appended) {
      view_producer_appended(block_count, count);
      waiting_reported = false;

      if (produce_delay > 0) {
        sleep(produce_delay);
      }
    } else {
      if (!waiting_reported) {
        view_producer_waiting();
        waiting_reported = true;
      }

      if (poll_delay > 0) {
        sleep(poll_delay);
      }
    }
  }

  view_producer_finished(block_count);

  ipc_semaphore_close(&ipc);
  ipc_memory_close(&ipc);
  ipc_semaphore_remove(ipc_ops, base_name);
  ipc_memory_remove(ipc_ops, base_name);
  return true;
}
