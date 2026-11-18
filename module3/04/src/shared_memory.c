#define _POSIX_C_SOURCE 202405L

#include "shared_memory.h"

#include <errno.h>

[[maybe_unused]] constexpr int SHARED_MEMORY_PROJECT_ID = 'M';

static key_t make_key(const char *key_path) {
  return ftok(key_path, SHARED_MEMORY_PROJECT_ID);
}

static bool attach(SharedMemory *memory) {
  void *addr = shmat(memory->shmid, nullptr, 0);

  if (addr == (void *)-1) {
    return false;
  }

  memory->segment = addr;
  return true;
}

bool shared_memory_create(SharedMemory *memory, const char *key_path,
                          const size_t data_capacity, bool *created_out) {
  if (memory == nullptr || key_path == nullptr || created_out == nullptr ||
      data_capacity == 0 || data_capacity > UINT32_MAX) {
    return false;
  }

  memory->key = make_key(key_path);

  if (memory->key == (key_t)-1) {
    return false;
  }

  const size_t total_size = sizeof(SharedSegment) + data_capacity;
  const auto shmid =
      shmget(memory->key, total_size, IPC_CREAT | IPC_EXCL | 0600);

  if (shmid == -1) {
    if (errno == EEXIST) {
      *created_out = false;
      return shared_memory_open(memory, key_path, data_capacity);
    }

    return false;
  }

  *created_out = true;
  memory->shmid = shmid;

  if (!attach(memory)) {
    shmctl(shmid, IPC_RMID, nullptr);
    return false;
  }

  memory->segment->data_capacity = (uint32_t)data_capacity;
  memory->segment->producer_finished = 0;
  return true;
}

bool shared_memory_open(SharedMemory *memory, const char *key_path,
                        const size_t data_capacity) {
  if (memory == nullptr || key_path == nullptr) {
    return false;
  }

  memory->key = make_key(key_path);

  if (memory->key == (key_t)-1) {
    return false;
  }

  const size_t total_size =
      data_capacity > 0 ? sizeof(SharedSegment) + data_capacity : 0;
  memory->shmid = shmget(memory->key, total_size, 0600);

  if (memory->shmid == -1) {
    return false;
  }

  return attach(memory);
}

void shared_memory_close(SharedMemory *memory) {
  if (memory == nullptr || memory->segment == nullptr) {
    return;
  }

  shmdt(memory->segment);
  memory->segment = nullptr;
  memory->shmid = -1;
}

bool shared_memory_remove(const char *key_path) {
  if (key_path == nullptr) {
    return false;
  }

  const auto key = make_key(key_path);

  if (key == (key_t)-1) {
    return false;
  }

  const auto shmid = shmget(key, 0, 0600);

  if (shmid == -1) {
    return false;
  }

  return shmctl(shmid, IPC_RMID, nullptr) == 0;
}

void *shared_memory_data(const SharedMemory *memory) {
  if (memory == nullptr || memory->segment == nullptr) {
    return nullptr;
  }

  return (char *)memory->segment + sizeof(SharedSegment);
}

size_t shared_memory_data_capacity(const SharedMemory *memory) {
  if (memory == nullptr || memory->segment == nullptr) {
    return 0;
  }

  return memory->segment->data_capacity;
}

bool shared_memory_is_producer_finished(const SharedMemory *memory) {
  if (memory == nullptr || memory->segment == nullptr) {
    return false;
  }

  return memory->segment->producer_finished != 0;
}

void shared_memory_set_producer_finished(SharedMemory *memory) {
  if (memory == nullptr || memory->segment == nullptr) {
    return;
  }

  memory->segment->producer_finished = 1;
}
