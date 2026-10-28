#include "permissions.h"

#include "permission_bits.h"
#include "permission_modifier.h"
#include "permission_stat.h"

PermissionResult permission_from_input(const char *input) {
  PermissionResult result = {.bits = 0,
                             .status = PERMISSION_STATUS_PARSE_ERROR};

  const PermissionParseResult parsed = permission_parse(input);

  if (!parsed.success) {
    return result;
  }

  result.bits = parsed.bits;
  result.status = PERMISSION_STATUS_OK;
  return result;
}

PermissionResult permission_from_file(const char *path) {
  PermissionResult result = {.bits = 0, .status = PERMISSION_STATUS_FILE_ERROR};

  const PermissionStatResult stat_result = permission_stat_file(path);

  if (!stat_result.success) {
    return result;
  }

  result.bits = stat_result.bits;
  result.status = PERMISSION_STATUS_OK;
  return result;
}

PermissionResult permission_from_modify(uint32_t current_bits,
                                        const char *command) {
  PermissionResult result = {.bits = 0,
                             .status = PERMISSION_STATUS_MODIFY_ERROR};

  const PermissionModifyResult modify_result =
      permission_modify(current_bits, command);

  if (!modify_result.success) {
    return result;
  }

  result.bits = modify_result.bits;
  result.status = PERMISSION_STATUS_OK;
  return result;
}

char *permission_format_symbolic(uint32_t bits) {
  return permission_to_symbolic(bits);
}

char *permission_format_octal(uint32_t bits) {
  return permission_to_octal(bits);
}

char *permission_format_binary(uint32_t bits) {
  return permission_to_binary(bits);
}