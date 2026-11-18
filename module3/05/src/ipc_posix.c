#define _POSIX_C_SOURCE 202405L

#include "ipc_posix.h"

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static bool posix_memory_open(void **context_out, const char *name);
static bool posix_semaphore_open(void **context_out, const char *name);

constexpr size_t OBJECT_NAME_CAPACITY = 128;

typedef struct {
  int fd;
  SharedSegment *segment;
} PosixMemory;

typedef struct {
  sem_t *handle;
} PosixSemaphore;

static void make_object_name(char *buffer, const size_t size,
                             const char *base_name, const char *suffix) {
  snprintf(buffer, size, "%s%s", base_name, suffix);
}

static bool map_segment(PosixMemory *memory, const size_t total_size) {
  void *addr = mmap(nullptr, total_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    memory->fd, 0);

  if (addr == MAP_FAILED) {
    return false;
  }

  memory->segment = addr;
  return true;
}

static bool posix_memory_create(void **context_out, const char *name,
                                const size_t data_capacity, bool *created_out) {
  if (context_out == nullptr || name == nullptr || created_out == nullptr ||
      data_capacity == 0 || data_capacity > UINT32_MAX) {
    return false;
  }

  char object_name[OBJECT_NAME_CAPACITY];
  make_object_name(object_name, sizeof(object_name), name, "_shm");

  const auto memory = (PosixMemory *)calloc(1, sizeof(PosixMemory));

  if (memory == nullptr) {
    return false;
  }

  memory->fd = shm_open(object_name, O_RDWR | O_CREAT | O_EXCL, 0600);

  if (memory->fd == -1) {
    free(memory);

    if (errno == EEXIST) {
      *created_out = false;
      return posix_memory_open(context_out, name);
    }

    return false;
  }

  *created_out = true;
  const size_t total_size = sizeof(SharedSegment) + data_capacity;

  if (ftruncate(memory->fd, (off_t)total_size) == -1 ||
      !map_segment(memory, total_size)) {
    close(memory->fd);
    shm_unlink(object_name);
    free(memory);
    return false;
  }

  memory->segment->data_capacity = (uint32_t)data_capacity;
  memory->segment->producer_finished = 0;
  *context_out = memory;
  return true;
}

static bool posix_memory_open(void **context_out, const char *name) {
  if (context_out == nullptr || name == nullptr) {
    return false;
  }

  char object_name[OBJECT_NAME_CAPACITY];
  make_object_name(object_name, sizeof(object_name), name, "_shm");

  const auto memory = (PosixMemory *)calloc(1, sizeof(PosixMemory));

  if (memory == nullptr) {
    return false;
  }

  memory->fd = shm_open(object_name, O_RDWR, 0600);

  if (memory->fd == -1) {
    free(memory);
    return false;
  }

  struct stat status = {0};

  if (fstat(memory->fd, &status) == -1 ||
      status.st_size < (off_t)sizeof(SharedSegment) ||
      !map_segment(memory, (size_t)status.st_size)) {
    close(memory->fd);
    free(memory);
    return false;
  }

  *context_out = memory;
  return true;
}

static void posix_memory_close(void *context) {
  if (context == nullptr) {
    return;
  }

  const auto memory = (PosixMemory *)context;
  const size_t total_size =
      sizeof(SharedSegment) + memory->segment->data_capacity;
  munmap(memory->segment, total_size);
  close(memory->fd);
  free(memory);
}

static bool posix_memory_remove(const char *name) {
  if (name == nullptr) {
    return false;
  }

  char object_name[OBJECT_NAME_CAPACITY];
  make_object_name(object_name, sizeof(object_name), name, "_shm");
  return shm_unlink(object_name) == 0;
}

static void *posix_memory_data(const void *context) {
  if (context == nullptr) {
    return nullptr;
  }

  const auto memory = (const PosixMemory *)context;
  return (char *)memory->segment + sizeof(SharedSegment);
}

static size_t posix_memory_data_capacity(const void *context) {
  if (context == nullptr) {
    return 0;
  }

  const auto memory = (const PosixMemory *)context;
  return memory->segment->data_capacity;
}

static bool posix_memory_is_producer_finished(const void *context) {
  if (context == nullptr) {
    return false;
  }

  const auto memory = (const PosixMemory *)context;
  return memory->segment->producer_finished != 0;
}

static void posix_memory_set_producer_finished(void *context) {
  if (context == nullptr) {
    return;
  }

  const auto memory = (PosixMemory *)context;
  memory->segment->producer_finished = 1;
}

static bool posix_semaphore_create(void **context_out, const char *name,
                                   bool *created_out) {
  if (context_out == nullptr || name == nullptr || created_out == nullptr) {
    return false;
  }

  char object_name[OBJECT_NAME_CAPACITY];
  make_object_name(object_name, sizeof(object_name), name, "_sem");

  const auto semaphore = (PosixSemaphore *)calloc(1, sizeof(PosixSemaphore));

  if (semaphore == nullptr) {
    return false;
  }

  semaphore->handle = sem_open(object_name, O_CREAT | O_EXCL, 0600, 1);

  if (semaphore->handle == SEM_FAILED) {
    free(semaphore);

    if (errno == EEXIST) {
      *created_out = false;
      return posix_semaphore_open(context_out, name);
    }

    return false;
  }

  *created_out = true;
  *context_out = semaphore;
  return true;
}

static bool posix_semaphore_open(void **context_out, const char *name) {
  if (context_out == nullptr || name == nullptr) {
    return false;
  }

  char object_name[OBJECT_NAME_CAPACITY];
  make_object_name(object_name, sizeof(object_name), name, "_sem");

  const auto semaphore = (PosixSemaphore *)calloc(1, sizeof(PosixSemaphore));

  if (semaphore == nullptr) {
    return false;
  }

  semaphore->handle = sem_open(object_name, 0);

  if (semaphore->handle == SEM_FAILED) {
    free(semaphore);
    return false;
  }

  *context_out = semaphore;
  return true;
}

static void posix_semaphore_close(void *context) {
  if (context == nullptr) {
    return;
  }

  const auto semaphore = (PosixSemaphore *)context;
  sem_close(semaphore->handle);
  free(semaphore);
}

static bool posix_semaphore_wait(const void *context) {
  if (context == nullptr) {
    return false;
  }

  const auto semaphore = (const PosixSemaphore *)context;

  while (sem_wait(semaphore->handle) == -1) {
    if (errno != EINTR) {
      return false;
    }
  }

  return true;
}

static bool posix_semaphore_post(const void *context) {
  if (context == nullptr) {
    return false;
  }

  const auto semaphore = (const PosixSemaphore *)context;
  return sem_post(semaphore->handle) == 0;
}

static bool posix_semaphore_remove(const char *name) {
  if (name == nullptr) {
    return false;
  }

  char object_name[OBJECT_NAME_CAPACITY];
  make_object_name(object_name, sizeof(object_name), name, "_sem");
  return sem_unlink(object_name) == 0;
}

static const IpcOps POSIX_OPS = {
    .memory_create = posix_memory_create,
    .memory_open = posix_memory_open,
    .memory_close = posix_memory_close,
    .memory_remove = posix_memory_remove,
    .memory_data = posix_memory_data,
    .memory_data_capacity = posix_memory_data_capacity,
    .memory_is_producer_finished = posix_memory_is_producer_finished,
    .memory_set_producer_finished = posix_memory_set_producer_finished,
    .semaphore_create = posix_semaphore_create,
    .semaphore_open = posix_semaphore_open,
    .semaphore_close = posix_semaphore_close,
    .semaphore_wait = posix_semaphore_wait,
    .semaphore_post = posix_semaphore_post,
    .semaphore_remove = posix_semaphore_remove,
};

const IpcOps *ipc_posix_ops() {
  return &POSIX_OPS;
}
