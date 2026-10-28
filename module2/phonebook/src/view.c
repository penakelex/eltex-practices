#include <stdio.h>
#include <view.h>

static void view_print_string_list(const char *label,
                                   const Vector *string_vector) {
  if (string_vector->count == 0) {
    return;
  }

  printf("    %s:\n", label);

  for (size_t index = 0; index < string_vector->count; ++index) {
    const char *value = *(char *const *)vector_at(string_vector, index);
    printf("      - %s\n", value);
  }
}

void view_print_contact(const Contact *contact, const size_t number) {
  printf("\n[%zu] %s %s", number, contact->last_name, contact->first_name);

  if (contact->middle_name != nullptr && contact->middle_name[0] != '\0') {
    printf(" %s", contact->middle_name);
  }

  putchar('\n');

  if (contact->workplace != nullptr || contact->position != nullptr) {
    printf("    Работа: %s, должность: %s\n",
           contact->workplace != nullptr ? contact->workplace : "-",
           contact->position != nullptr ? contact->position : "-");
  }

  view_print_string_list("Телефоны", &contact->phones);
  view_print_string_list("Email", &contact->emails);
  view_print_string_list("Соцсети", &contact->social_networks);
  view_print_string_list("Мессенджеры", &contact->messengers);
}

bool view_print_phonebook(const Phonebook *phonebook) {
  if (phonebook->contacts.count == 0) {
    puts("Список пуст.");
    return false;
  }

  for (size_t index = 0; index < phonebook->contacts.count; ++index) {
    const Contact *contact =
        (const Contact *)vector_at(&phonebook->contacts, index);
    view_print_contact(contact, index + 1);
  }

  return true;
}