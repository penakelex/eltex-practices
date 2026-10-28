#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  bool success;
  uint32_t bits;
} PermissionStatResult;

PermissionStatResult permission_stat_file(const char *path);