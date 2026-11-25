#pragma once

#include "ipc.h"

#include <sys/ipc.h>

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

bool system_v_memory_create(void **context_out, const char *name,
                            size_t data_capacity, bool *created_out);

bool system_v_memory_open(void **context_out, const char *name);

void system_v_memory_close(void *context);

bool system_v_memory_remove(const char *name);

void *system_v_memory_data(const void *context);

size_t system_v_memory_data_capacity(const void *context);

bool system_v_memory_is_producer_finished(const void *context);

void system_v_memory_set_producer_finished(void *context);

bool system_v_semaphore_create(void **context_out, const char *name,
                               bool *created_out);

bool system_v_semaphore_open(void **context_out, const char *name);

void system_v_semaphore_close(void *context);

bool system_v_semaphore_wait(const void *context);

bool system_v_semaphore_post(const void *context);

bool system_v_semaphore_remove(const char *name);
