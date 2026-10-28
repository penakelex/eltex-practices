#include <phonebook.h>

static void free_contact_element(void *element) {
  contact_free((Contact *)element);
}

void phonebook_initialize(Phonebook *phonebook) {
  vector_initialize(&phonebook->contacts, sizeof(Contact),
                    free_contact_element);
}

void phonebook_free(Phonebook *phonebook) {
  vector_free(&phonebook->contacts);
}

bool phonebook_add(Phonebook *phonebook, Contact *contact) {
  if (!vector_push_back(&phonebook->contacts, contact)) {
    return false;
  }
  contact_initialize(contact);
  return true;
}

bool phonebook_remove(Phonebook *phonebook, const size_t index) {
  return vector_remove_at(&phonebook->contacts, index);
}