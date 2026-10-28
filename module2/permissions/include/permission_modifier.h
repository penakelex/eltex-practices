#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  bool success;
  uint32_t bits;
} PermissionModifyResult;

PermissionModifyResult permission_modify(uint32_t current_bits,
                                         const char *command);