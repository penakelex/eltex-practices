#define _POSIX_C_SOURCE 202405L

#include "client_registry.h"

#include <stdlib.h>
#include <string.h>

#include "tcp_socket.h"

constexpr size_t CLIENT_REGISTRY_INITIAL_CAPACITY = 8;

void client_registry_initialize(ClientRegistry *registry) {
  if (registry == nullptr) {
    return;
  }

  registry->fds = nullptr;
  registry->size = 0;
  registry->capacity = 0;
}

void client_registry_free(ClientRegistry *registry) {
  if (registry == nullptr) {
    return;
  }

  free(registry->fds);
  client_registry_initialize(registry);
}

bool client_registry_add(ClientRegistry *registry, const int fd) {
  if (registry == nullptr) {
    return false;
  }

  if (registry->size == registry->capacity) {
    const size_t new_capacity = registry->capacity == 0
                                    ? CLIENT_REGISTRY_INITIAL_CAPACITY
                                    : registry->capacity * 2;
    int *const new_fds = realloc(registry->fds, new_capacity * sizeof(int));

    if (new_fds == nullptr) {
      return false;
    }

    registry->fds = new_fds;
    registry->capacity = new_capacity;
  }

  registry->fds[registry->size] = fd;
  registry->size += 1;
  return true;
}

bool client_registry_remove(ClientRegistry *registry, const int fd) {
  if (registry == nullptr) {
    return false;
  }

  for (size_t index = 0; index < registry->size; index += 1) {
    if (registry->fds[index] == fd) {
      registry->fds[index] = registry->fds[registry->size - 1];
      registry->size -= 1;
      return true;
    }
  }

  return false;
}

bool client_registry_contains(const ClientRegistry *registry, const int fd) {
  if (registry == nullptr) {
    return false;
  }

  for (size_t index = 0; index < registry->size; index += 1) {
    if (registry->fds[index] == fd) {
      return true;
    }
  }

  return false;
}

size_t client_registry_size(const ClientRegistry *registry) {
  if (registry == nullptr) {
    return 0;
  }

  return registry->size;
}

void client_registry_broadcast(const ClientRegistry *registry,
                               const int excluded_fd, const void *data,
                               const size_t size) {
  if (registry == nullptr || data == nullptr) {
    return;
  }

  for (size_t index = 0; index < registry->size; index += 1) {
    if (registry->fds[index] == excluded_fd) {
      continue;
    }

    tcp_socket_send_all(registry->fds[index], data, size);
  }
}
