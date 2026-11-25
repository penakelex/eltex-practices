#define _POSIX_C_SOURCE 202405L

#include "ipc_system_v_internal.h"

static const IpcOps SYSTEM_V_OPS = {
    .memory_create = system_v_memory_create,
    .memory_open = system_v_memory_open,
    .memory_close = system_v_memory_close,
    .memory_remove = system_v_memory_remove,
    .memory_data = system_v_memory_data,
    .memory_data_capacity = system_v_memory_data_capacity,
    .memory_is_producer_finished = system_v_memory_is_producer_finished,
    .memory_set_producer_finished = system_v_memory_set_producer_finished,
    .semaphore_create = system_v_semaphore_create,
    .semaphore_open = system_v_semaphore_open,
    .semaphore_close = system_v_semaphore_close,
    .semaphore_wait = system_v_semaphore_wait,
    .semaphore_post = system_v_semaphore_post,
    .semaphore_remove = system_v_semaphore_remove,
};

const IpcOps *ipc_system_v_ops() {
  return &SYSTEM_V_OPS;
}
