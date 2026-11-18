#pragma once

#include <sys/ipc.h>
#include <sys/sem.h>

typedef struct {
  key_t key;
  int semid;
} Semaphore;

bool semaphore_create(Semaphore *semaphore, const char *key_path,
                      const int project_id, bool *created_out);

bool semaphore_open(Semaphore *semaphore, const char *key_path,
                    const int project_id);

bool semaphore_wait(const Semaphore *semaphore);

bool semaphore_post(const Semaphore *semaphore);

bool semaphore_remove(const char *key_path, const int project_id);
