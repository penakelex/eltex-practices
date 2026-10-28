#pragma once

#include <stdint.h>

typedef enum {
  PERMISSION_STATUS_OK = 0,
  PERMISSION_STATUS_PARSE_ERROR,
  PERMISSION_STATUS_FILE_ERROR,
  PERMISSION_STATUS_MODIFY_ERROR,
} PermissionStatus;

typedef struct {
  uint32_t bits;
  PermissionStatus status;
} PermissionResult;

PermissionResult permission_from_input(const char *input);
PermissionResult permission_from_file(const char *path);
PermissionResult permission_from_modify(uint32_t current_bits,
                                        const char *command);

char *permission_format_symbolic(uint32_t bits);
char *permission_format_octal(uint32_t bits);
char *permission_format_binary(uint32_t bits);