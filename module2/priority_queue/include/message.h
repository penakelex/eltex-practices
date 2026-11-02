#pragma once

#include <stddef.h>

enum : size_t { PRIORITY_LEVELS = 256 };

typedef struct {
  unsigned int priority;
  unsigned int id;
  char payload[32];
} Message;

void message_print(const Message *message);