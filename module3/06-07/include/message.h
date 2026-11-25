#pragma once

#include <stddef.h>

[[maybe_unused]] constexpr size_t CHAT_INPUT_PATH_CAPACITY = 512;

typedef enum : int {
  CHAT_INPUT_EMPTY,
  CHAT_INPUT_TEXT,
  CHAT_INPUT_FILE,
  CHAT_INPUT_UNKNOWN_COMMAND
} ChatInputType;

typedef struct {
  ChatInputType type;
  char text[1024];
  char file_path[CHAT_INPUT_PATH_CAPACITY];
} ChatInput;

bool chat_input_parse(const char *line, ChatInput *out);
