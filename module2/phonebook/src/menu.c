#include <fields.h>
#include <input_utils.h>
#include <menu.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string_utils.h>
#include <view.h>

static void edit_single_string_field(const char *label, char **field,
                                     const bool required) {
  char buffer[INPUT_LINE_BUFFER_SIZE];

  while (true) {
    prompt_for_input(label, buffer, sizeof buffer);

    if (buffer[0] == '\0') {
      if (required && *field == nullptr) {
        puts("Это поле обязательно для заполнения.");
        continue;
      }

      if (!required) {
        free(*field);
        *field = nullptr;
      }

      return;
    }

    const auto new_value = duplicate_string(buffer);

    if (new_value == nullptr) {
      puts("Ошибка выделения памяти.");
      return;
    }

    free(*field);
    *field = new_value;
    return;
  }
}

static void edit_string_list(const char *label, Vector *string_vector) {
  vector_clear(string_vector);
  printf("%s (пустая строка — завершить ввод):\n", label);
  char buffer[INPUT_LINE_BUFFER_SIZE];

  while (true) {
    prompt_for_input("  > ", buffer, sizeof buffer);

    if (buffer[0] == '\0') {
      break;
    }

    const auto copy = duplicate_string(buffer);

    if (copy == nullptr) {
      puts("Ошибка выделения памяти.");
      break;
    }

    if (!vector_push_back(string_vector, &copy)) {
      free(copy);
      puts("Ошибка выделения памяти.");
      break;
    }
  }
}

static void edit_selected_fields(Contact *contact, const uint32_t field_mask) {
  if (field_mask & FIELD_LAST_NAME) {
    edit_single_string_field("Фамилия (обязательно): ", &contact->last_name,
                             true);
  }

  if (field_mask & FIELD_FIRST_NAME) {
    edit_single_string_field("Имя (обязательно): ", &contact->first_name, true);
  }

  if (field_mask & FIELD_MIDDLE_NAME) {
    edit_single_string_field(
        "Отчество (Enter — пропустить): ", &contact->middle_name, false);
  }

  if (field_mask & FIELD_WORKPLACE) {
    edit_single_string_field(
        "Место работы (Enter — пропустить): ", &contact->workplace, false);
  }

  if (field_mask & FIELD_POSITION) {
    edit_single_string_field(
        "Должность (Enter — пропустить): ", &contact->position, false);
  }

  if (field_mask & FIELD_PHONES) {
    edit_string_list("Телефоны", &contact->phones);
  }

  if (field_mask & FIELD_EMAILS) {
    edit_string_list("Email", &contact->emails);
  }

  if (field_mask & FIELD_SOCIAL_NETWORKS) {
    edit_string_list("Соцсети", &contact->social_networks);
  }

  if (field_mask & FIELD_MESSENGERS) {
    edit_string_list("Мессенджеры", &contact->messengers);
  }
}

static void add_contact(Phonebook *phonebook) {
  Contact contact;
  contact_initialize(&contact);
  puts("Заполните данные нового контакта:");
  edit_selected_fields(&contact, FIELD_ALL);

  if (!phonebook_add(phonebook, &contact)) {
    contact_free(&contact);
    puts("Ошибка добавления контакта.");
    return;
  }

  puts("Контакт добавлен.");
}

static void edit_contact(Phonebook *phonebook) {
  if (!view_print_phonebook(phonebook)) {
    return;
  }

  const uint32_t number = ask_for_uint32("Номер контакта (0 — отмена): ", 0,
                                         (uint32_t)phonebook->contacts.count);

  if (number == 0) {
    return;
  }

  Contact *const contact =
      (Contact *)vector_at(&phonebook->contacts, (size_t)(number - 1));

  puts("\nКакие поля редактировать?");
  puts("  0. Все поля");
  puts("  1. Фамилия");
  puts("  2. Имя");
  puts("  3. Отчество");
  puts("  4. Место работы");
  puts("  5. Должность");
  puts("  6. Телефоны");
  puts("  7. Email");
  puts("  8. Соцсети");
  puts("  9. Мессенджеры");
  puts("Введите номера через запятую, например: 1,3,5");
  puts("Введите 0 для редактирования всех полей.");

  char buffer[INPUT_LINE_BUFFER_SIZE];

  while (true) {
    prompt_for_input("Выбор: ", buffer, sizeof buffer);
    const uint32_t field_mask = fields_parse_selection(buffer);

    if (field_mask != 0) {
      edit_selected_fields(contact, field_mask);
      puts("Изменения применены.");
      return;
    }

    puts("Некорректный ввод, попробуйте ещё раз.");
  }
}

static void delete_contact(Phonebook *phonebook) {
  if (!view_print_phonebook(phonebook)) {
    return;
  }

  const uint32_t number =
      ask_for_uint32("Номер контакта для удаления (0 — отмена): ", 0,
                     (uint32_t)phonebook->contacts.count);

  if (number == 0) {
    return;
  }

  if (phonebook_remove(phonebook, (size_t)(number - 1))) {
    puts("Контакт удалён.");
  } else {
    puts("Не удалось удалить контакт.");
  }
}

void menu_run(Phonebook *phonebook) {
  while (true) {
    puts("\n=== Телефонная книга ===");
    puts("1. Показать все контакты");
    puts("2. Добавить контакт");
    puts("3. Редактировать контакт");
    puts("4. Удалить контакт");
    puts("0. Выход");

    const uint32_t choice = ask_for_uint32("Выбор: ", 0, 4);

    switch (choice) {
    case 0: {
      return;
    }
    case 1: {
      view_print_phonebook(phonebook);
      break;
    }
    case 2: {
      add_contact(phonebook);
      break;
    }
    case 3: {
      edit_contact(phonebook);
      break;
    }
    case 4: {
      delete_contact(phonebook);
      break;
    }
    default: {
      break;
    }
    }
  }
}