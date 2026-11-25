#pragma once

#include "ipc.h"

#include <semaphore.h>

constexpr size_t OBJECT_NAME_CAPACITY = 128;

typedef struct {
  int fd;
  SharedSegment *segment;
} PosixMemory;

typedef struct {
  sem_t *handle;
} PosixSemaphore;

void posix_make_object_name(char *buffer, size_t size, const char *base_name,
                            const char *suffix);

bool posix_memory_create(void **context_out, const char *name,
                         size_t data_capacity, bool *created_out);

bool posix_memory_open(void **context_out, const char *name);

void posix_memory_close(void *context);

bool posix_memory_remove(const char *name);

void *posix_memory_data(const void *context);

size_t posix_memory_data_capacity(const void *context);

bool posix_memory_is_producer_finished(const void *context);

void posix_memory_set_producer_finished(void *context);

bool posix_semaphore_create(void **context_out, const char *name,
                            bool *created_out);

bool posix_semaphore_open(void **context_out, const char *name);

void posix_semaphore_close(void *context);

bool posix_semaphore_wait(const void *context);

bool posix_semaphore_post(const void *context);

bool posix_semaphore_remove(const char *name);
