#pragma once

#include <collection.h>
#include <contact.h>
#include <stddef.h>

typedef struct {
  void *data;
  const CollectionOperations *operations;
} Phonebook;

bool phonebook_initialize(Phonebook *phonebook, const char *collection_name);
void phonebook_free(Phonebook *phonebook);

bool phonebook_add(Phonebook *phonebook, Contact *contact);
size_t phonebook_count(const Phonebook *phonebook);
Contact *phonebook_at(Phonebook *phonebook, size_t index);
bool phonebook_remove(Phonebook *phonebook, size_t index);
bool phonebook_reorder_element(Phonebook *phonebook, size_t index);
void phonebook_dump(Phonebook *phonebook, ContactVisitor visitor,
                    void *context);
void phonebook_dump_tree(Phonebook *phonebook, TreeVisitor visitor,
                         void *context);
bool phonebook_is_hierarchical(const Phonebook *phonebook);