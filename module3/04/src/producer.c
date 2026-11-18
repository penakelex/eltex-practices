#define _POSIX_C_SOURCE 202405L

#include "producer.h"

#include "block_list.h"
#include "semaphore.h"
#include "shared_memory.h"
#include "view.h"

#include <stdlib.h>
#include <time.h>
#include <unistd.h>

constexpr int SEMAPHORE_PROJECT_ID = 'S';
constexpr uint32_t PRODUCER_BLOCK_MAX_VALUES = 64;

static uint32_t random_block_size() {
  return (uint32_t)rand() % PRODUCER_BLOCK_MAX_VALUES + 1;
}

static void fill_random_values(int32_t *values, uint32_t count) {
  for (uint32_t index = 0; index < count; ++index) {
    values[index] = rand();
  }
}

bool producer_run(const char *key_path, const size_t data_capacity,
                  const unsigned int produce_delay,
                  const unsigned int poll_delay) {
  SharedMemory memory;
  bool created = false;

  if (!shared_memory_create(&memory, key_path, data_capacity, &created)) {
    view_error("cannot create shared memory");
    return false;
  }

  if (!created) {
    view_shared_memory_exists();
    shared_memory_close(&memory);
    return false;
  }

  Semaphore semaphore;

  if (!semaphore_create(&semaphore, key_path, SEMAPHORE_PROJECT_ID, &created) ||
      !created) {
    view_error("cannot create semaphore");
    shared_memory_close(&memory);
    shared_memory_remove(key_path);
    return false;
  }

  if (!block_list_initialize(shared_memory_data(&memory),
                             shared_memory_data_capacity(&memory))) {
    view_error("cannot initialize block list");
    semaphore_remove(key_path, SEMAPHORE_PROJECT_ID);
    shared_memory_close(&memory);
    shared_memory_remove(key_path);
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

    void *data = shared_memory_data(&memory);
    const size_t capacity = shared_memory_data_capacity(&memory);

    semaphore_wait(&semaphore);

    uint32_t offset = 0;
    const auto appended =
        block_list_append(data, capacity, values, count, &offset);
    bool finished = false;

    if (appended) {
      block_count += 1;
    } else if (block_list_all_processed(data, capacity)) {
      shared_memory_set_producer_finished(&memory);
      finished = true;
    }

    semaphore_post(&semaphore);

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

  semaphore_remove(key_path, SEMAPHORE_PROJECT_ID);
  shared_memory_close(&memory);
  shared_memory_remove(key_path);
  return true;
}
