#include <command.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static const Command **commands = nullptr;
static size_t commands_capacity = 0;
static size_t commands_count = 0;

bool command_register(const Command *command) {
  if (command == nullptr || command->name == nullptr) {
    return false;
  }

  if (commands_count == commands_capacity) {
    const size_t new_capacity =
        (commands_capacity == 0) ? 32 : commands_capacity * 2;
    const Command **resized = (const Command **)realloc(
        (void *)commands, new_capacity * sizeof *resized);

    if (resized == nullptr) {
      return false;
    }

    commands = resized;
    commands_capacity = new_capacity;
  }

  commands[commands_count++] = command;
  return true;
}

void command_reset(void) {
  free((void *)commands);
  commands = nullptr;
  commands_capacity = 0;
  commands_count = 0;
}

const Command *command_find(const char *name) {
  if (name == nullptr) {
    return nullptr;
  }

  for (size_t index = 0; index < commands_count; ++index) {
    if (strcmp(commands[index]->name, name) == 0) {
      return commands[index];
    }
  }

  return nullptr;
}

const Command *const *command_all(size_t *count) {
  if (count != nullptr) {
    *count = commands_count;
  }

  if (commands_count == 0) {
    return nullptr;
  }

  return commands;
}