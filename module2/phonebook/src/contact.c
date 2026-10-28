#include <contact.h>
#include <stdlib.h>

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