#pragma once

#include <stddef.h>

typedef struct {
  void **handles;
  size_t count;
  size_t capacity;
} CommandLoader;

bool command_loader_load_directory(CommandLoader *loader,
                                   const char *directory);
bool command_loader_reload(CommandLoader *loader, const char *directory);
void command_loader_unload(CommandLoader *loader);