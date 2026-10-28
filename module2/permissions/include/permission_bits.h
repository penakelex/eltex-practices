#pragma once

#include <stdbool.h>
#include <stdint.h>

[[maybe_unused]] static constexpr uint32_t PERMISSION_MASK = 0x0FFFu;
[[maybe_unused]] static constexpr uint32_t PERMISSION_BIT_COUNT = 12u;

typedef struct {
  bool success;
  uint32_t bits;
} PermissionParseResult;

PermissionParseResult permission_parse(const char *input);
PermissionParseResult permission_parse_octal(const char *input);
PermissionParseResult permission_parse_symbolic(const char *input);

char *permission_to_symbolic(uint32_t bits);
char *permission_to_octal(uint32_t bits);
char *permission_to_binary(uint32_t bits);