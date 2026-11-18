#define _POSIX_C_SOURCE 202405L

#include "semaphore.h"

static key_t make_key(const char *key_path, const int project_id) {
  return ftok(key_path, project_id);
}

bool semaphore_create(Semaphore *semaphore, const char *key_path,
                      const int project_id, bool *created_out) {
  if (semaphore == nullptr || key_path == nullptr || created_out == nullptr) {
    return false;
  }

  semaphore->key = make_key(key_path, project_id);

  if (semaphore->key == (key_t)-1) {
    return false;
  }

  const auto semid = semget(semaphore->key, 1, IPC_CREAT | IPC_EXCL | 0600);

  if (semid == -1) {
    *created_out = false;
    return semaphore_open(semaphore, key_path, project_id);
  }

  *created_out = true;
  semaphore->semid = semid;

  union semun {
    int value;
    struct semid_ds *buffer;
    unsigned short *array;
  } argument = {.value = 1};

  return semctl(semid, 0, SETVAL, argument) == 0;
}

bool semaphore_open(Semaphore *semaphore, const char *key_path,
                    const int project_id) {
  if (semaphore == nullptr || key_path == nullptr) {
    return false;
  }

  semaphore->key = make_key(key_path, project_id);

  if (semaphore->key == (key_t)-1) {
    return false;
  }

  semaphore->semid = semget(semaphore->key, 1, 0600);
  return semaphore->semid != -1;
}

bool semaphore_wait(const Semaphore *semaphore) {
  if (semaphore == nullptr) {
    return false;
  }

  struct sembuf operation = {.sem_num = 0, .sem_op = -1, .sem_flg = 0};
  return semop(semaphore->semid, &operation, 1) == 0;
}

bool semaphore_post(const Semaphore *semaphore) {
  if (semaphore == nullptr) {
    return false;
  }

  struct sembuf operation = {.sem_num = 0, .sem_op = 1, .sem_flg = 0};
  return semop(semaphore->semid, &operation, 1) == 0;
}

bool semaphore_remove(const char *key_path, const int project_id) {
  if (key_path == nullptr) {
    return false;
  }

  const auto key = make_key(key_path, project_id);

  if (key == (key_t)-1) {
    return false;
  }

  const auto semid = semget(key, 1, 0600);

  if (semid == -1) {
    return false;
  }

  return semctl(semid, 0, IPC_RMID) == 0;
}
