#pragma once

#include <message.h>
#include <stddef.h>

enum : size_t { CIRCULAR_BUFFER_CAPACITY = 64 };

typedef struct {
  Message items[CIRCULAR_BUFFER_CAPACITY];
  size_t head;
  size_t tail;
  size_t count;
} CircularBuffer;

void circular_buffer_initialize(CircularBuffer *buffer);
void circular_buffer_clear(CircularBuffer *buffer);

void circular_buffer_push(CircularBuffer *buffer, const Message *message);
bool circular_buffer_pop(CircularBuffer *buffer, Message *out);

bool circular_buffer_is_empty(const CircularBuffer *buffer);
bool circular_buffer_is_full(const CircularBuffer *buffer);
size_t circular_buffer_count(const CircularBuffer *buffer);