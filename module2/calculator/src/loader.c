#define _POSIX_C_SOURCE 200809L

#include <command.h>
#include <dirent.h>
#include <dlfcn.h>
#include <loader.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef const Command *(*CommandExport)(void);

static bool loader_push_handle(CommandLoader *loader, void *handle) {
  if (loader->count == loader->capacity) {
    const size_t new_capacity =
        (loader->capacity == 0) ? 8 : loader->capacity * 2;
    void **resized = realloc(loader->handles, new_capacity * sizeof *resized);

    if (resized == nullptr) {
      return false;
    }

    loader->handles = resized;
    loader->capacity = new_capacity;
  }

  loader->handles[loader->count++] = handle;
  return true;
}

static void loader_process_entry(CommandLoader *loader, const char *directory,
                                 const char *name) {
  const size_t name_length = strlen(name);

  if (name_length < 4 || strcmp(name + name_length - 3, ".so") != 0) {
    return;
  }

  char path[4096];
  const int written = snprintf(path, sizeof path, "%s/%s", directory, name);

  if (written < 0 || (size_t)written >= sizeof path) {
    return;
  }

  void *handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);

  if (handle == nullptr) {
    fprintf(stderr, "dlopen %s: %s\n", path, dlerror());
    return;
  }

  CommandExport export_function =
      (CommandExport)dlsym(handle, "command_export");

  if (export_function == nullptr) {
    fprintf(stderr, "dlsym %s: %s\n", path, dlerror());
    dlclose(handle);
    return;
  }

  const Command *command = export_function();

  if (command == nullptr || !command_register(command)) {
    fprintf(stderr, "register %s: failed\n", path);
    dlclose(handle);
    return;
  }

  if (!loader_push_handle(loader, handle)) {
    dlclose(handle);
  }
}

bool command_loader_load_directory(CommandLoader *loader,
                                   const char *directory) {
  if (loader == nullptr || directory == nullptr) {
    return false;
  }

  DIR *dir = opendir(directory);

  if (dir == nullptr) {
    return false;
  }

  struct dirent *entry = nullptr;

  while ((entry = readdir(dir)) != nullptr) {
    loader_process_entry(loader, directory, entry->d_name);
  }

  closedir(dir);
  return true;
}

void command_loader_unload(CommandLoader *loader) {
  if (loader == nullptr) {
    return;
  }

  for (size_t index = 0; index < loader->count; ++index) {
    dlclose(loader->handles[index]);
  }

  free(loader->handles);
  loader->handles = nullptr;
  loader->count = 0;
  loader->capacity = 0;
}

bool command_loader_reload(CommandLoader *loader, const char *directory) {
  if (loader == nullptr || directory == nullptr) {
    return false;
  }

  command_reset();
  command_loader_unload(loader);
  return command_loader_load_directory(loader, directory);
}