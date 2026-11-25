#define _POSIX_C_SOURCE 202405L

#include "ipc_posix_internal.h"

#include <fcntl.h>

#include <errno.h>
#include <stdlib.h>

bool posix_semaphore_create(void **context_out, const char *name,
                            bool *created_out) {
  if (context_out == nullptr || name == nullptr || created_out == nullptr) {
    return false;
  }

  char object_name[OBJECT_NAME_CAPACITY];
  posix_make_object_name(object_name, sizeof(object_name), name, "_sem");

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

bool posix_semaphore_open(void **context_out, const char *name) {
  if (context_out == nullptr || name == nullptr) {
    return false;
  }

  char object_name[OBJECT_NAME_CAPACITY];
  posix_make_object_name(object_name, sizeof(object_name), name, "_sem");

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

void posix_semaphore_close(void *context) {
  if (context == nullptr) {
    return;
  }

  const auto semaphore = (PosixSemaphore *)context;
  sem_close(semaphore->handle);
  free(semaphore);
}

bool posix_semaphore_wait(const void *context) {
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

bool posix_semaphore_post(const void *context) {
  if (context == nullptr) {
    return false;
  }

  const auto semaphore = (const PosixSemaphore *)context;
  return sem_post(semaphore->handle) == 0;
}

bool posix_semaphore_remove(const char *name) {
  if (name == nullptr) {
    return false;
  }

  char object_name[OBJECT_NAME_CAPACITY];
  posix_make_object_name(object_name, sizeof(object_name), name, "_sem");
  return sem_unlink(object_name) == 0;
}
