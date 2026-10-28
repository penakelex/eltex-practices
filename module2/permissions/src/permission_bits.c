#include "permission_bits.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool parse_symbolic_position(const char *input, size_t pos,
                                    uint32_t *bits) {
  const char c = input[pos];
  const uint32_t mask = 1u << (8u - (uint32_t)pos);
  const size_t class_index = pos % 3;

  if (class_index == 0) {
    if (c == 'r') {
      *bits |= mask;
      return true;
    }

    return c == '-';
  }

  if (class_index == 1) {
    if (c == 'w') {
      *bits |= mask;
      return true;
    }

    return c == '-';
  }

  if (c == 'x') {
    *bits |= mask;
    return true;
  }

  if (c == '-') {
    return true;
  }

  if (pos == 2) {
    if (c == 's') {
      *bits |= mask | (1u << 11);
      return true;
    }

    if (c == 'S') {
      *bits |= 1u << 11;
      return true;
    }
  }

  if (pos == 5) {
    if (c == 's') {
      *bits |= mask | (1u << 10);
      return true;
    }

    if (c == 'S') {
      *bits |= 1u << 10;
      return true;
    }
  }

  if (pos == 8) {
    if (c == 't') {
      *bits |= mask | (1u << 9);
      return true;
    }

    if (c == 'T') {
      *bits |= 1u << 9;
      return true;
    }
  }

  return false;
}

PermissionParseResult permission_parse_octal(const char *input) {
  PermissionParseResult result = {.success = false, .bits = 0};

  if (input == nullptr) {
    return result;
  }

  auto const length = strlen(input);

  if (length < 1 || length > 4) {
    return result;
  }

  uint32_t bits = 0;

  for (size_t i = 0; i < length; i++) {
    const char c = input[i];

    if (c < '0' || c > '7') {
      return result;
    }

    bits = (bits << 3) | (uint32_t)(c - '0');
  }

  if (bits > PERMISSION_MASK) {
    return result;
  }

  result.success = true;
  result.bits = bits;
  return result;
}

PermissionParseResult permission_parse_symbolic(const char *input) {
  PermissionParseResult result = {.success = false, .bits = 0};

  if (input == nullptr) {
    return result;
  }

  auto const length = strlen(input);

  if (length != 9) {
    return result;
  }

  uint32_t bits = 0;

  for (size_t i = 0; i < 9; i++) {
    if (!parse_symbolic_position(input, i, &bits)) {
      return result;
    }
  }

  result.success = true;
  result.bits = bits;
  return result;
}

PermissionParseResult permission_parse(const char *input) {
  PermissionParseResult result = {.success = false, .bits = 0};

  if (input == nullptr) {
    return result;
  }

  auto const length = strlen(input);

  if (length == 0) {
    return result;
  }

  bool all_octal = true;

  for (size_t i = 0; i < length; i++) {
    if (input[i] < '0' || input[i] > '7') {
      all_octal = false;
      break;
    }
  }

  if (all_octal) {
    return permission_parse_octal(input);
  }

  return permission_parse_symbolic(input);
}

char *permission_to_symbolic(uint32_t bits) {
  char *result = malloc(10);

  if (result == nullptr) {
    return nullptr;
  }

  const bool user_exec = (bits & (1u << 6)) != 0;
  const bool group_exec = (bits & (1u << 3)) != 0;
  const bool other_exec = (bits & (1u << 0)) != 0;
  const bool setuid = (bits & (1u << 11)) != 0;
  const bool setgid = (bits & (1u << 10)) != 0;
  const bool sticky = (bits & (1u << 9)) != 0;

  result[0] = (bits & (1u << 8)) ? 'r' : '-';
  result[1] = (bits & (1u << 7)) ? 'w' : '-';

  if (setuid) {
    result[2] = user_exec ? 's' : 'S';
  } else {
    result[2] = user_exec ? 'x' : '-';
  }

  result[3] = (bits & (1u << 5)) ? 'r' : '-';
  result[4] = (bits & (1u << 4)) ? 'w' : '-';

  if (setgid) {
    result[5] = group_exec ? 's' : 'S';
  } else {
    result[5] = group_exec ? 'x' : '-';
  }

  result[6] = (bits & (1u << 2)) ? 'r' : '-';
  result[7] = (bits & (1u << 1)) ? 'w' : '-';

  if (sticky) {
    result[8] = other_exec ? 't' : 'T';
  } else {
    result[8] = other_exec ? 'x' : '-';
  }

  result[9] = '\0';
  return result;
}

char *permission_to_octal(uint32_t bits) {
  char buffer[8];
  const uint32_t masked = bits & PERMISSION_MASK;

  int written;

  if (masked <= 0777u) {
    written = snprintf(buffer, sizeof(buffer), "%03o", (unsigned)masked);
  } else {
    written = snprintf(buffer, sizeof(buffer), "%04o", (unsigned)masked);
  }

  if (written < 0) {
    return nullptr;
  }

  char *result = malloc((size_t)written + 1);

  if (result == nullptr) {
    return nullptr;
  }

  memcpy(result, buffer, (size_t)written + 1);
  return result;
}

char *permission_to_binary(uint32_t bits) {
  char *result = malloc(16);

  if (result == nullptr) {
    return nullptr;
  }

  const uint32_t masked = bits & PERMISSION_MASK;

  for (int group = 0; group < 4; group++) {
    for (int bit = 0; bit < 3; bit++) {
      const int bit_index = 11 - (group * 3 + bit);
      result[group * 4 + bit] = ((masked >> bit_index) & 1u) ? '1' : '0';
    }

    if (group < 3) {
      result[group * 4 + 3] = ' ';
    }
  }

  result[15] = '\0';
  return result;
}