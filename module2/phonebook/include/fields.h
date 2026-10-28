#pragma once

#include <stdint.h>

enum : uint32_t {
  FIELD_LAST_NAME = 1u << 0,
  FIELD_FIRST_NAME = 1u << 1,
  FIELD_MIDDLE_NAME = 1u << 2,
  FIELD_WORKPLACE = 1u << 3,
  FIELD_POSITION = 1u << 4,
  FIELD_PHONES = 1u << 5,
  FIELD_EMAILS = 1u << 6,
  FIELD_SOCIAL_NETWORKS = 1u << 7,
  FIELD_MESSENGERS = 1u << 8,
  FIELD_ALL = (1u << 9) - 1,
};

uint32_t fields_parse_selection(const char *input);