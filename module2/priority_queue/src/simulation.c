#include <message.h>
#include <priority_queue.h>
#include <simulation.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned int next_id = 0;

unsigned int simulation_next_id(void) {
  return next_id++;
}

void simulation_generate(PriorityQueue *queue, unsigned int count,
                         unsigned int maximum_priority) {
  for (unsigned int index = 0; index < count; ++index) {
    Message message;
    message.priority = (unsigned int)(rand() % (int)(maximum_priority + 1));
    message.id = simulation_next_id();
    snprintf(message.payload, sizeof message.payload, "msg-%u", message.id);
    priority_queue_add(queue, &message);
  }
}

void simulation_extract_all(PriorityQueue *queue) {
  puts("Извлечение всех сообщений:");
  Message message;
  unsigned int previous_priority = (unsigned int)PRIORITY_LEVELS;
  size_t extracted = 0;
  bool sorted = true;

  while (priority_queue_extract_first(queue, &message)) {
    message_print(&message);

    if (message.priority > previous_priority) {
      sorted = false;
    }

    previous_priority = message.priority;
    ++extracted;
  }

  printf("Извлечено: %zu\n", extracted);

  if (extracted == 0) {
    puts("Очередь была пуста.");
  } else if (sorted) {
    puts("Порядок корректен (приоритеты не возрастают).");
  } else {
    puts("ВНИМАНИЕ: приоритеты шли не по убыванию.");
  }
}

void simulation_extract_priority(PriorityQueue *queue, unsigned int priority) {
  printf("Извлечение сообщений с приоритетом %u:\n", priority);
  Message message;
  size_t extracted = 0;

  while (priority_queue_extract_with_priority(queue, priority, &message)) {
    message_print(&message);
    ++extracted;
  }

  printf("Извлечено: %zu\n", extracted);
}

void simulation_extract_at_least(PriorityQueue *queue, unsigned int threshold) {
  printf("Извлечение сообщений с приоритетом не ниже %u:\n", threshold);
  Message message;
  size_t extracted = 0;

  while (priority_queue_extract_with_priority_at_least(queue, threshold,
                                                       &message)) {
    message_print(&message);
    ++extracted;
  }

  printf("Извлечено: %zu\n", extracted);
}