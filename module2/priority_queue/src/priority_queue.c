#include <priority_queue.h>

void priority_queue_initialize(PriorityQueue *queue) {
  for (size_t index = 0; index < PRIORITY_LEVELS; ++index) {
    circular_buffer_initialize(&queue->levels[index]);
  }

  queue->total = 0;
}

void priority_queue_clear(PriorityQueue *queue) {
  priority_queue_initialize(queue);
}

void priority_queue_add(PriorityQueue *queue, const Message *message) {
  unsigned int priority = message->priority;

  if (priority >= PRIORITY_LEVELS) {
    priority = PRIORITY_LEVELS - 1;
  }

  CircularBuffer *level = &queue->levels[priority];
  bool was_full = circular_buffer_is_full(level);
  circular_buffer_push(level, message);

  if (!was_full) {
    ++queue->total;
  }
}

bool priority_queue_extract_first(PriorityQueue *queue, Message *out) {
  for (size_t index = PRIORITY_LEVELS; index > 0; --index) {
    if (circular_buffer_pop(&queue->levels[index - 1], out)) {
      --queue->total;
      return true;
    }
  }

  return false;
}

bool priority_queue_extract_with_priority(PriorityQueue *queue,
                                          unsigned int priority, Message *out) {
  if (priority >= PRIORITY_LEVELS) {
    return false;
  }

  if (circular_buffer_pop(&queue->levels[priority], out)) {
    --queue->total;
    return true;
  }

  return false;
}

bool priority_queue_extract_with_priority_at_least(PriorityQueue *queue,
                                                   unsigned int threshold,
                                                   Message *out) {
  if (queue == nullptr || out == nullptr || threshold >= PRIORITY_LEVELS) {
    return false;
  }

  size_t best_priority = PRIORITY_LEVELS;
  size_t best_distance = PRIORITY_LEVELS;

  for (size_t priority = PRIORITY_LEVELS; priority > 0; --priority) {
    const size_t level = priority - 1;

    if (circular_buffer_is_empty(&queue->levels[level])) {
      continue;
    }

    const size_t distance =
        (level > threshold) ? (level - threshold) : (threshold - level);

    if (distance < best_distance) {
      best_distance = distance;
      best_priority = level;
    }
  }

  if (best_priority == PRIORITY_LEVELS) {
    return false;
  }

  if (!circular_buffer_pop(&queue->levels[best_priority], out)) {
    return false;
  }

  --queue->total;
  return true;
}

bool priority_queue_is_empty(const PriorityQueue *queue) {
  return queue->total == 0;
}

size_t priority_queue_count(const PriorityQueue *queue) {
  return queue->total;
}

size_t priority_queue_count_with_priority(const PriorityQueue *queue,
                                          unsigned int priority) {
  if (priority >= PRIORITY_LEVELS) {
    return 0;
  }

  return circular_buffer_count(&queue->levels[priority]);
}