#define _POSIX_C_SOURCE 202405L

#include "ipc.h"

void ipc_initialize(Ipc *ipc, const IpcOps *ops) {
  if (ipc == nullptr) {
    return;
  }

  ipc->memory = nullptr;
  ipc->semaphore = nullptr;
  ipc->ops = ops;
}

bool ipc_memory_create(Ipc *ipc, const char *name, const size_t data_capacity,
                       bool *created_out) {
  if (ipc == nullptr || ipc->ops == nullptr || created_out == nullptr) {
    return false;
  }

  return ipc->ops->memory_create(&ipc->memory, name, data_capacity,
                                 created_out);
}

bool ipc_memory_open(Ipc *ipc, const char *name) {
  if (ipc == nullptr || ipc->ops == nullptr) {
    return false;
  }

  return ipc->ops->memory_open(&ipc->memory, name);
}

void ipc_memory_close(Ipc *ipc) {
  if (ipc == nullptr || ipc->ops == nullptr || ipc->memory == nullptr) {
    return;
  }

  ipc->ops->memory_close(ipc->memory);
  ipc->memory = nullptr;
}

bool ipc_memory_remove(const IpcOps *ops, const char *name) {
  if (ops == nullptr || name == nullptr) {
    return false;
  }

  return ops->memory_remove(name);
}

void *ipc_memory_data(const Ipc *ipc) {
  if (ipc == nullptr || ipc->ops == nullptr || ipc->memory == nullptr) {
    return nullptr;
  }

  return ipc->ops->memory_data(ipc->memory);
}

size_t ipc_memory_data_capacity(const Ipc *ipc) {
  if (ipc == nullptr || ipc->ops == nullptr || ipc->memory == nullptr) {
    return 0;
  }

  return ipc->ops->memory_data_capacity(ipc->memory);
}

bool ipc_memory_is_producer_finished(const Ipc *ipc) {
  if (ipc == nullptr || ipc->ops == nullptr || ipc->memory == nullptr) {
    return false;
  }

  return ipc->ops->memory_is_producer_finished(ipc->memory);
}

void ipc_memory_set_producer_finished(Ipc *ipc) {
  if (ipc == nullptr || ipc->ops == nullptr || ipc->memory == nullptr) {
    return;
  }

  ipc->ops->memory_set_producer_finished(ipc->memory);
}

bool ipc_semaphore_create(Ipc *ipc, const char *name, bool *created_out) {
  if (ipc == nullptr || ipc->ops == nullptr || created_out == nullptr) {
    return false;
  }

  return ipc->ops->semaphore_create(&ipc->semaphore, name, created_out);
}

bool ipc_semaphore_open(Ipc *ipc, const char *name) {
  if (ipc == nullptr || ipc->ops == nullptr) {
    return false;
  }

  return ipc->ops->semaphore_open(&ipc->semaphore, name);
}

void ipc_semaphore_close(Ipc *ipc) {
  if (ipc == nullptr || ipc->ops == nullptr || ipc->semaphore == nullptr) {
    return;
  }

  ipc->ops->semaphore_close(ipc->semaphore);
  ipc->semaphore = nullptr;
}

bool ipc_semaphore_wait(const Ipc *ipc) {
  if (ipc == nullptr || ipc->ops == nullptr || ipc->semaphore == nullptr) {
    return false;
  }

  return ipc->ops->semaphore_wait(ipc->semaphore);
}

bool ipc_semaphore_post(const Ipc *ipc) {
  if (ipc == nullptr || ipc->ops == nullptr || ipc->semaphore == nullptr) {
    return false;
  }

  return ipc->ops->semaphore_post(ipc->semaphore);
}

bool ipc_semaphore_remove(const IpcOps *ops, const char *name) {
  if (ops == nullptr || name == nullptr) {
    return false;
  }

  return ops->semaphore_remove(name);
}
