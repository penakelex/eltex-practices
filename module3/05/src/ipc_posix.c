#define _POSIX_C_SOURCE 202405L

#include "ipc_posix_internal.h"

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
