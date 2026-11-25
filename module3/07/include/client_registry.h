#pragma once

#include <stddef.h>

typedef struct {
  int *fds;
  size_t size;
  size_t capacity;
} ClientRegistry;

void client_registry_initialize(ClientRegistry *registry);
void client_registry_free(ClientRegistry *registry);

bool client_registry_add(ClientRegistry *registry, const int fd);
bool client_registry_remove(ClientRegistry *registry, const int fd);
bool client_registry_contains(const ClientRegistry *registry, const int fd);
size_t client_registry_size(const ClientRegistry *registry);

void client_registry_broadcast(const ClientRegistry *registry,
                               const int excluded_fd, const void *data,
                               const size_t size);
