#pragma once

#include <contact.h>
#include <stdbool.h>
#include <stddef.h>
#include <vector.h>

typedef struct {
  Vector contacts; // elements: Contact
} Phonebook;

void phonebook_initialize(Phonebook *phonebook);
void phonebook_free(Phonebook *phonebook);
bool phonebook_add(Phonebook *phonebook, Contact *contact);
bool phonebook_remove(Phonebook *phonebook, size_t index);