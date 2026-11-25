#define _POSIX_C_SOURCE 202405L

#include "ipc_system_v_internal.h"

#include <sys/shm.h>

#include <errno.h>
#include <stdlib.h>

static bool memory_attach(SystemVMemory *memory) {
  void *addr = shmat(memory->shmid, nullptr, 0);

  if (addr == (void *)-1) {
    return false;
  }

  memory->segment = addr;
  return true;
}

bool system_v_memory_create(void **context_out, const char *name,
                            const size_t data_capacity, bool *created_out) {
  if (context_out == nullptr || name == nullptr || created_out == nullptr ||
      data_capacity == 0 || data_capacity > UINT32_MAX) {
    return false;
  }

  const auto memory = (SystemVMemory *)calloc(1, sizeof(SystemVMemory));

  if (memory == nullptr) {
    return false;
  }

  memory->key = ftok(name, MEMORY_PROJECT_ID);

  if (memory->key == (key_t)-1) {
    free(memory);
    return false;
  }

  const size_t total_size = sizeof(SharedSegment) + data_capacity;
  memory->shmid = shmget(memory->key, total_size, IPC_CREAT | IPC_EXCL | 0600);

  if (memory->shmid == -1) {
    free(memory);

    if (errno == EEXIST) {
      *created_out = false;
      return system_v_memory_open(context_out, name);
    }

    return false;
  }

  *created_out = true;

  if (!memory_attach(memory)) {
    shmctl(memory->shmid, IPC_RMID, nullptr);
    free(memory);
    return false;
  }

  memory->segment->data_capacity = (uint32_t)data_capacity;
  memory->segment->producer_finished = 0;
  *context_out = memory;
  return true;
}

bool system_v_memory_open(void **context_out, const char *name) {
  if (context_out == nullptr || name == nullptr) {
    return false;
  }

  const auto memory = (SystemVMemory *)calloc(1, sizeof(SystemVMemory));

  if (memory == nullptr) {
    return false;
  }

  memory->key = ftok(name, MEMORY_PROJECT_ID);

  if (memory->key == (key_t)-1) {
    free(memory);
    return false;
  }

  memory->shmid = shmget(memory->key, 0, 0600);

  if (memory->shmid == -1 || !memory_attach(memory)) {
    free(memory);
    return false;
  }

  *context_out = memory;
  return true;
}

void system_v_memory_close(void *context) {
  if (context == nullptr) {
    return;
  }

  const auto memory = (SystemVMemory *)context;
  shmdt(memory->segment);
  free(memory);
}

bool system_v_memory_remove(const char *name) {
  if (name == nullptr) {
    return false;
  }

  const auto key = ftok(name, MEMORY_PROJECT_ID);

  if (key == (key_t)-1) {
    return false;
  }

  const auto shmid = shmget(key, 0, 0600);

  if (shmid == -1) {
    return false;
  }

  return shmctl(shmid, IPC_RMID, nullptr) == 0;
}

void *system_v_memory_data(const void *context) {
  if (context == nullptr) {
    return nullptr;
  }

  const auto memory = (const SystemVMemory *)context;
  return (char *)memory->segment + sizeof(SharedSegment);
}

size_t system_v_memory_data_capacity(const void *context) {
  if (context == nullptr) {
    return 0;
  }

  const auto memory = (const SystemVMemory *)context;
  return memory->segment->data_capacity;
}

bool system_v_memory_is_producer_finished(const void *context) {
  if (context == nullptr) {
    return false;
  }

  const auto memory = (const SystemVMemory *)context;
  return memory->segment->producer_finished != 0;
}

void system_v_memory_set_producer_finished(void *context) {
  if (context == nullptr) {
    return;
  }

  const auto memory = (SystemVMemory *)context;
  memory->segment->producer_finished = 1;
}
