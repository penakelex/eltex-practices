#include <input_utils.h>
#include <menu.h>
#include <message.h>
#include <priority_queue.h>
#include <simulation.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void menu_show_state(const PriorityQueue *queue) {
  printf("Всего сообщений: %zu\n", priority_queue_count(queue));

  for (size_t index = 0; index < PRIORITY_LEVELS; ++index) {
    size_t level_count =
        priority_queue_count_with_priority(queue, (unsigned int)index);

    if (level_count > 0) {
      printf("  приоритет %zu: %zu\n", index, level_count);
    }
  }
}

static void menu_generate(PriorityQueue *queue) {
  uint32_t count =
      ask_for_uint32("Сколько сообщений сгенерировать (1-1000): ", 1, 1000);
  simulation_generate(queue, (unsigned int)count,
                      (unsigned int)(PRIORITY_LEVELS - 1));
  printf("Сгенерировано %u сообщений.\n", count);
}

static void menu_add_manually(PriorityQueue *queue) {
  Message message;
  message.priority =
      ask_for_uint32("Приоритет (0-255): ", 0, (uint32_t)(PRIORITY_LEVELS - 1));
  message.id = simulation_next_id();

  char buffer[INPUT_LINE_BUFFER_SIZE];
  prompt_for_input("Текст: ", buffer, sizeof buffer);
  snprintf(message.payload, sizeof message.payload, "%s", buffer);

  priority_queue_add(queue, &message);
  puts("Сообщение добавлено.");
}

static void menu_extract_first(PriorityQueue *queue) {
  Message message;

  if (priority_queue_extract_first(queue, &message)) {
    message_print(&message);
  } else {
    puts("Очередь пуста.");
  }
}

static void menu_extract_priority(PriorityQueue *queue) {
  uint32_t priority =
      ask_for_uint32("Приоритет (0-255): ", 0, (uint32_t)(PRIORITY_LEVELS - 1));
  Message message;

  if (priority_queue_extract_with_priority(queue, priority, &message)) {
    message_print(&message);
  } else {
    puts("На этом приоритете ничего нет.");
  }
}

static void menu_extract_at_least(PriorityQueue *queue) {
  uint32_t threshold =
      ask_for_uint32("Порог (0-255): ", 0, (uint32_t)(PRIORITY_LEVELS - 1));
  Message message;

  if (priority_queue_extract_with_priority_at_least(queue, threshold,
                                                    &message)) {
    message_print(&message);
  } else {
    puts("Нет сообщений с приоритетом не ниже заданного.");
  }
}

void menu_run(PriorityQueue *queue) {
  while (true) {
    puts("\n=== Очередь с приоритетом ===");
    puts("1. Показать состояние");
    puts("2. Сгенерировать N сообщений");
    puts("3. Добавить сообщение вручную");
    puts("4. Извлечь первым в очереди");
    puts("5. Извлечь с указанным приоритетом");
    puts("6. Извлечь с приоритетом не ниже заданного");
    puts("0. Выход");

    uint32_t choice = ask_for_uint32("Выбор: ", 0, 6);

    switch (choice) {
    case 0: {
      return;
    }
    case 1: {
      menu_show_state(queue);
      break;
    }
    case 2: {
      menu_generate(queue);
      break;
    }
    case 3: {
      menu_add_manually(queue);
      break;
    }
    case 4: {
      menu_extract_first(queue);
      break;
    }
    case 5: {
      menu_extract_priority(queue);
      break;
    }
    case 6: {
      menu_extract_at_least(queue);
      break;
    }
    default: {
      break;
    }
    }
  }
}