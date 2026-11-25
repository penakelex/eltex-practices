#define _POSIX_C_SOURCE 202405L

#include "message.h"

#include <string.h>

#include "protocol.h"

static void trim_end(char *line) {
  const size_t length = strlen(line);

  if (length > 0 && line[length - 1] == '\n') {
    line[length - 1] = '\0';
  }
}

bool chat_input_parse(const char *line, ChatInput *out) {
  if (line == nullptr || out == nullptr) {
    return false;
  }

  char buffer[CHAT_TEXT_CAPACITY];
  const size_t length = strlen(line);

  if (length >= sizeof(buffer)) {
    out->type = CHAT_INPUT_EMPTY;
    return true;
  }

  memcpy(buffer, line, length + 1);
  trim_end(buffer);

  if (buffer[0] == '\0') {
    out->type = CHAT_INPUT_EMPTY;
    return true;
  }

  if (buffer[0] != '/') {
    out->type = CHAT_INPUT_TEXT;
    memcpy(out->text, buffer, strlen(buffer) + 1);
    return true;
  }

  static const char file_command[] = "/file ";

  if (strncmp(buffer, file_command, strlen(file_command)) == 0) {
    const char *path = buffer + strlen(file_command);

    if (path[0] == '\0' || strlen(path) >= CHAT_INPUT_PATH_CAPACITY) {
      out->type = CHAT_INPUT_EMPTY;
      return true;
    }

    out->type = CHAT_INPUT_FILE;
    memcpy(out->file_path, path, strlen(path) + 1);
    return true;
  }

  out->type = CHAT_INPUT_UNKNOWN_COMMAND;
  return true;
}
