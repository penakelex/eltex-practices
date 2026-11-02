#include <circular_buffer.h>

void circular_buffer_initialize(CircularBuffer *buffer) {
  buffer->head = 0;
  buffer->tail = 0;
  buffer->count = 0;
}

void circular_buffer_clear(CircularBuffer *buffer) {
  buffer->head = 0;
  buffer->tail = 0;
  buffer->count = 0;
}

void circular_buffer_push(CircularBuffer *buffer, const Message *message) {
  if (buffer->count == CIRCULAR_BUFFER_CAPACITY) {
    buffer->head = (buffer->head + 1) % CIRCULAR_BUFFER_CAPACITY;
    --buffer->count;
  }

  buffer->items[buffer->tail] = *message;
  buffer->tail = (buffer->tail + 1) % CIRCULAR_BUFFER_CAPACITY;
  ++buffer->count;
}

bool circular_buffer_pop(CircularBuffer *buffer, Message *out) {
  if (buffer->count == 0) {
    return false;
  }

  *out = buffer->items[buffer->head];
  buffer->head = (buffer->head + 1) % CIRCULAR_BUFFER_CAPACITY;
  --buffer->count;
  return true;
}

bool circular_buffer_is_empty(const CircularBuffer *buffer) {
  return buffer->count == 0;
}

bool circular_buffer_is_full(const CircularBuffer *buffer) {
  return buffer->count == CIRCULAR_BUFFER_CAPACITY;
}

size_t circular_buffer_count(const CircularBuffer *buffer) {
  return buffer->count;
}