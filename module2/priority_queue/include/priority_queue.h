#pragma once

#include <circular_buffer.h>
#include <message.h>
#include <stddef.h>

typedef struct {
  CircularBuffer levels[PRIORITY_LEVELS];
  size_t total;
} PriorityQueue;

void priority_queue_initialize(PriorityQueue *queue);
void priority_queue_clear(PriorityQueue *queue);

void priority_queue_add(PriorityQueue *queue, const Message *message);

bool priority_queue_extract_first(PriorityQueue *queue, Message *out);
bool priority_queue_extract_with_priority(PriorityQueue *queue,
                                          unsigned int priority, Message *out);
bool priority_queue_extract_with_priority_at_least(PriorityQueue *queue,
                                                   unsigned int threshold,
                                                   Message *out);

bool priority_queue_is_empty(const PriorityQueue *queue);
size_t priority_queue_count(const PriorityQueue *queue);
size_t priority_queue_count_with_priority(const PriorityQueue *queue,
                                          unsigned int priority);