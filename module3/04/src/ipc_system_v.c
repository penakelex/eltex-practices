#define _POSIX_C_SOURCE 202405L

#include "ipc_system_v.h"

#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <stdlib.h>

static bool system_v_memory_open(void **context_out, const char *name);
static bool system_v_semaphore_open(void **context_out, const char *name);

constexpr int MEMORY_PROJECT_ID = 'M';
constexpr int SEMAPHORE_PROJECT_ID = 'S';

typedef struct {
  key_t key;
  int shmid;
  SharedSegment *segment;
} SystemVMemory;

typedef struct {
  key_t key;
  int semid;
} SystemVSemaphore;

static bool memory_attach(SystemVMemory *memory) {
  void *addr = shmat(memory->shmid, nullptr, 0);

  if (addr == (void *)-1) {
    return false;
  }

  memory->segment = addr;
  return true;
}

static bool system_v_memory_create(void **context_out, const char *name,
                                   const size_t data_capacity,
                                   bool *created_out) {
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

static bool system_v_memory_open(void **context_out, const char *name) {
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

static void system_v_memory_close(void *context) {
  if (context == nullptr) {
    return;
  }

  const auto memory = (SystemVMemory *)context;
  shmdt(memory->segment);
  free(memory);
}

static bool system_v_memory_remove(const char *name) {
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

static void *system_v_memory_data(const void *context) {
  if (context == nullptr) {
    return nullptr;
  }

  const auto memory = (const SystemVMemory *)context;
  return (char *)memory->segment + sizeof(SharedSegment);
}

static size_t system_v_memory_data_capacity(const void *context) {
  if (context == nullptr) {
    return 0;
  }

  const auto memory = (const SystemVMemory *)context;
  return memory->segment->data_capacity;
}

static bool system_v_memory_is_producer_finished(const void *context) {
  if (context == nullptr) {
    return false;
  }

  const auto memory = (const SystemVMemory *)context;
  return memory->segment->producer_finished != 0;
}

static void system_v_memory_set_producer_finished(void *context) {
  if (context == nullptr) {
    return;
  }

  const auto memory = (SystemVMemory *)context;
  memory->segment->producer_finished = 1;
}

static bool system_v_semaphore_create(void **context_out, const char *name,
                                      bool *created_out) {
  if (context_out == nullptr || name == nullptr || created_out == nullptr) {
    return false;
  }

  const auto semaphore =
      (SystemVSemaphore *)calloc(1, sizeof(SystemVSemaphore));

  if (semaphore == nullptr) {
    return false;
  }

  semaphore->key = ftok(name, SEMAPHORE_PROJECT_ID);

  if (semaphore->key == (key_t)-1) {
    free(semaphore);
    return false;
  }

  semaphore->semid = semget(semaphore->key, 1, IPC_CREAT | IPC_EXCL | 0600);

  if (semaphore->semid == -1) {
    free(semaphore);

    if (errno == EEXIST) {
      *created_out = false;
      return system_v_semaphore_open(context_out, name);
    }

    return false;
  }

  *created_out = true;

  union semun {
    int value;
    struct semid_ds *buffer;
    unsigned short *array;
  } argument = {.value = 1};

  if (semctl(semaphore->semid, 0, SETVAL, argument) == -1) {
    semctl(semaphore->semid, 0, IPC_RMID);
    free(semaphore);
    return false;
  }

  *context_out = semaphore;
  return true;
}

static bool system_v_semaphore_open(void **context_out, const char *name) {
  if (context_out == nullptr || name == nullptr) {
    return false;
  }

  const auto semaphore =
      (SystemVSemaphore *)calloc(1, sizeof(SystemVSemaphore));

  if (semaphore == nullptr) {
    return false;
  }

  semaphore->key = ftok(name, SEMAPHORE_PROJECT_ID);

  if (semaphore->key == (key_t)-1) {
    free(semaphore);
    return false;
  }

  semaphore->semid = semget(semaphore->key, 1, 0600);

  if (semaphore->semid == -1) {
    free(semaphore);
    return false;
  }

  *context_out = semaphore;
  return true;
}

static void system_v_semaphore_close(void *context) {
  free(context);
}

static bool system_v_semaphore_wait(const void *context) {
  if (context == nullptr) {
    return false;
  }

  const auto semaphore = (const SystemVSemaphore *)context;
  struct sembuf operation = {.sem_num = 0, .sem_op = -1, .sem_flg = 0};
  return semop(semaphore->semid, &operation, 1) == 0;
}

static bool system_v_semaphore_post(const void *context) {
  if (context == nullptr) {
    return false;
  }

  const auto semaphore = (const SystemVSemaphore *)context;
  struct sembuf operation = {.sem_num = 0, .sem_op = 1, .sem_flg = 0};
  return semop(semaphore->semid, &operation, 1) == 0;
}

static bool system_v_semaphore_remove(const char *name) {
  if (name == nullptr) {
    return false;
  }

  const auto key = ftok(name, SEMAPHORE_PROJECT_ID);

  if (key == (key_t)-1) {
    return false;
  }

  const auto semid = semget(key, 1, 0600);

  if (semid == -1) {
    return false;
  }

  return semctl(semid, 0, IPC_RMID) == 0;
}

static const IpcOps SYSTEM_V_OPS = {
    .memory_create = system_v_memory_create,
    .memory_open = system_v_memory_open,
    .memory_close = system_v_memory_close,
    .memory_remove = system_v_memory_remove,
    .memory_data = system_v_memory_data,
    .memory_data_capacity = system_v_memory_data_capacity,
    .memory_is_producer_finished = system_v_memory_is_producer_finished,
    .memory_set_producer_finished = system_v_memory_set_producer_finished,
    .semaphore_create = system_v_semaphore_create,
    .semaphore_open = system_v_semaphore_open,
    .semaphore_close = system_v_semaphore_close,
    .semaphore_wait = system_v_semaphore_wait,
    .semaphore_post = system_v_semaphore_post,
    .semaphore_remove = system_v_semaphore_remove,
};

const IpcOps *ipc_system_v_ops() {
  return &SYSTEM_V_OPS;
}
