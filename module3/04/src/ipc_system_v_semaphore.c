#define _POSIX_C_SOURCE 202405L

#include "ipc_system_v_internal.h"

#include <sys/sem.h>

#include <errno.h>
#include <stdlib.h>

bool system_v_semaphore_create(void **context_out, const char *name,
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

bool system_v_semaphore_open(void **context_out, const char *name) {
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

void system_v_semaphore_close(void *context) {
  free(context);
}

bool system_v_semaphore_wait(const void *context) {
  if (context == nullptr) {
    return false;
  }

  const auto semaphore = (const SystemVSemaphore *)context;
  struct sembuf operation = {.sem_num = 0, .sem_op = -1, .sem_flg = 0};
  return semop(semaphore->semid, &operation, 1) == 0;
}

bool system_v_semaphore_post(const void *context) {
  if (context == nullptr) {
    return false;
  }

  const auto semaphore = (const SystemVSemaphore *)context;
  struct sembuf operation = {.sem_num = 0, .sem_op = 1, .sem_flg = 0};
  return semop(semaphore->semid, &operation, 1) == 0;
}

bool system_v_semaphore_remove(const char *name) {
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
