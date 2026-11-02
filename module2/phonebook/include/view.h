#pragma once

#include <phonebook.h>
#include <stddef.h>

void view_print_contact(const Contact *contact, size_t number);
bool view_print_phonebook(Phonebook *phonebook);
bool view_print_phonebook_tree(Phonebook *phonebook);