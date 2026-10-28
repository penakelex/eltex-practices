#pragma once

#include <vector.h>

typedef struct {
  char *last_name;
  char *first_name;
  char *middle_name;
  char *workplace;
  char *position;
  Vector phones;          // elements: char*
  Vector emails;          // elements: char*
  Vector social_networks; // elements: char*
  Vector messengers;      // elements: char*
} Contact;

void contact_initialize(Contact *contact);
void contact_free(Contact *contact);