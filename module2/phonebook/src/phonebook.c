#include <phonebook.h>

bool phonebook_initialize(Phonebook *phonebook, const char *collection_name) {
  const CollectionOperations *operations =
      collection_get_operations(collection_name);

  if (operations == nullptr) {
    return false;
  }

  void *data = operations->create();

  if (data == nullptr) {
    return false;
  }

  phonebook->data = data;
  phonebook->operations = operations;
  return true;
}

void phonebook_free(Phonebook *phonebook) {
  if (phonebook->operations != nullptr && phonebook->data != nullptr) {
    phonebook->operations->destroy(phonebook->data);
  }

  phonebook->data = nullptr;
  phonebook->operations = nullptr;
}

bool phonebook_add(Phonebook *phonebook, Contact *contact) {
  if (!phonebook->operations->add(phonebook->data, contact)) {
    return false;
  }

  contact_initialize(contact);
  return true;
}

size_t phonebook_count(const Phonebook *phonebook) {
  return phonebook->operations->count(phonebook->data);
}

Contact *phonebook_at(Phonebook *phonebook, size_t index) {
  return phonebook->operations->at(phonebook->data, index);
}

bool phonebook_remove(Phonebook *phonebook, size_t index) {
  return phonebook->operations->remove_at(phonebook->data, index);
}

void phonebook_reorder(Phonebook *phonebook) {
  if (phonebook->operations != nullptr &&
      phonebook->operations->reorder != nullptr) {
    phonebook->operations->reorder(phonebook->data);
  }
}

void phonebook_dump(Phonebook *phonebook, ContactVisitor visitor,
                    void *context) {
  phonebook->operations->dump(phonebook->data, visitor, context);
}

void phonebook_dump_tree(Phonebook *phonebook, TreeVisitor visitor,
                         void *context) {
  if (phonebook->operations->dump_tree != nullptr) {
    phonebook->operations->dump_tree(phonebook->data, visitor, context);
  }
}

bool phonebook_is_hierarchical(const Phonebook *phonebook) {
  return phonebook->operations->dump_tree != nullptr;
}