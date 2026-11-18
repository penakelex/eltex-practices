#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct {
  uint32_t data_capacity;
  uint32_t producer_finished;
} SharedSegment;

typedef struct {
  key_t key;
  int shmid;
  SharedSegment *segment;
} SharedMemory;

bool shared_memory_create(SharedMemory *memory, const char *key_path,
                          const size_t data_capacity, bool *created_out);

bool shared_memory_open(SharedMemory *memory, const char *key_path,
                        const size_t data_capacity);

void shared_memory_close(SharedMemory *memory);

bool shared_memory_remove(const char *key_path);

void *shared_memory_data(const SharedMemory *memory);

size_t shared_memory_data_capacity(const SharedMemory *memory);

bool shared_memory_is_producer_finished(const SharedMemory *memory);

void shared_memory_set_producer_finished(SharedMemory *memory);
