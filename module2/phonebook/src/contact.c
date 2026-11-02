#include <contact.h>
#include <stdlib.h>
#include <string.h>

static void free_string_element(void *element) {
  free(*(char **)element);
}

void contact_initialize(Contact *contact) {
  contact->last_name = nullptr;
  contact->first_name = nullptr;
  contact->middle_name = nullptr;
  contact->workplace = nullptr;
  contact->position = nullptr;
  vector_initialize(&contact->phones, sizeof(char *), free_string_element);
  vector_initialize(&contact->emails, sizeof(char *), free_string_element);
  vector_initialize(&contact->social_networks, sizeof(char *),
                    free_string_element);
  vector_initialize(&contact->messengers, sizeof(char *), free_string_element);
}

void contact_free(Contact *contact) {
  free(contact->last_name);
  free(contact->first_name);
  free(contact->middle_name);
  free(contact->workplace);
  free(contact->position);
  vector_free(&contact->phones);
  vector_free(&contact->emails);
  vector_free(&contact->social_networks);
  vector_free(&contact->messengers);
  contact_initialize(contact);
}

static int compare_string(const char *left, const char *right) {
  return strcmp(left != nullptr ? left : "", right != nullptr ? right : "");
}

int contact_compare(const Contact *left, const Contact *right) {
  int result = compare_string(left->last_name, right->last_name);

  if (result != 0) {
    return result;
  }

  result = compare_string(left->first_name, right->first_name);

  if (result != 0) {
    return result;
  }

  return compare_string(left->middle_name, right->middle_name);
}