#include <phonebook.h>
#include <stdio.h>
#include <view.h>

typedef struct {
  bool printed_any;
} PhonebookPrintState;

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

static void view_print_visitor(const Contact *contact, const size_t index,
                               void *context) {
  PhonebookPrintState *state = context;
  state->printed_any = true;
  view_print_contact(contact, index + 1);
}

static void view_print_tree_visitor(const Contact *contact, const char *prefix,
                                    void *context) {
  PhonebookPrintState *state = context;
  state->printed_any = true;
  printf("%s%s %s\n", prefix, contact->last_name, contact->first_name);
}

bool view_print_phonebook(Phonebook *phonebook) {
  PhonebookPrintState state = {.printed_any = false};
  phonebook_dump(phonebook, view_print_visitor, &state);

  if (!state.printed_any) {
    puts("Список пуст.");
    return false;
  }

  return true;
}

bool view_print_phonebook_tree(Phonebook *phonebook) {
  if (!phonebook_is_hierarchical(phonebook)) {
    puts("Эта коллекция не является иерархической, вывожу плоский список.");
    return view_print_phonebook(phonebook);
  }

  PhonebookPrintState state = {.printed_any = false};
  phonebook_dump_tree(phonebook, view_print_tree_visitor, &state);

  if (!state.printed_any) {
    puts("Список пуст.");
    return false;
  }

  return true;
}