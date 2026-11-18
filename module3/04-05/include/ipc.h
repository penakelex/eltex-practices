#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint32_t data_capacity;
  uint32_t producer_finished;
} SharedSegment;

typedef struct IpcOps IpcOps;

typedef struct {
  void *memory;
  void *semaphore;
  const IpcOps *ops;
} Ipc;

struct IpcOps {
  bool (*memory_create)(void **context_out, const char *name,
                        size_t data_capacity, bool *created_out);
  bool (*memory_open)(void **context_out, const char *name);
  void (*memory_close)(void *context);
  bool (*memory_remove)(const char *name);
  void *(*memory_data)(const void *context);
  size_t (*memory_data_capacity)(const void *context);
  bool (*memory_is_producer_finished)(const void *context);
  void (*memory_set_producer_finished)(void *context);
  bool (*semaphore_create)(void **context_out, const char *name,
                           bool *created_out);
  bool (*semaphore_open)(void **context_out, const char *name);
  void (*semaphore_close)(void *context);
  bool (*semaphore_wait)(const void *context);
  bool (*semaphore_post)(const void *context);
  bool (*semaphore_remove)(const char *name);
};

void ipc_initialize(Ipc *ipc, const IpcOps *ops);

bool ipc_memory_create(Ipc *ipc, const char *name, size_t data_capacity,
                       bool *created_out);

bool ipc_memory_open(Ipc *ipc, const char *name);

void ipc_memory_close(Ipc *ipc);

bool ipc_memory_remove(const IpcOps *ops, const char *name);

void *ipc_memory_data(const Ipc *ipc);

size_t ipc_memory_data_capacity(const Ipc *ipc);

bool ipc_memory_is_producer_finished(const Ipc *ipc);

void ipc_memory_set_producer_finished(Ipc *ipc);

bool ipc_semaphore_create(Ipc *ipc, const char *name, bool *created_out);

bool ipc_semaphore_open(Ipc *ipc, const char *name);

void ipc_semaphore_close(Ipc *ipc);

bool ipc_semaphore_wait(const Ipc *ipc);

bool ipc_semaphore_post(const Ipc *ipc);

bool ipc_semaphore_remove(const IpcOps *ops, const char *name);
