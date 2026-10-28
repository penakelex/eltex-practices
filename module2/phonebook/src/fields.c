#include <fields.h>
#include <stdlib.h>

uint32_t fields_parse_selection(const char *input) {
  uint32_t field_mask = 0;
  const char *cursor = input;

  while (*cursor != '\0') {
    while (*cursor == ' ' || *cursor == ',' || *cursor == '\t') {
      ++cursor;
    }

    if (*cursor == '\0') {
      break;
    }

    char *end_pointer = nullptr;
    const unsigned long value = strtoul(cursor, &end_pointer, 10);

    if (end_pointer == cursor) {
      return 0;
    }

    if (value > 9) {
      return 0;
    }

    if (value == 0) {
      return FIELD_ALL;
    }

    const uint32_t bit_index = (uint32_t)(value - 1);
    field_mask |= 1u << bit_index;
    cursor = end_pointer;
  }

  return field_mask;
}