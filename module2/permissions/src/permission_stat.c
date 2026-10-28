#include "permission_stat.h"

#include "permission_bits.h"

#include <sys/stat.h>

PermissionStatResult permission_stat_file(const char *path) {
  PermissionStatResult result = {.success = false, .bits = 0};

  if (path == nullptr) {
    return result;
  }

  struct stat info;

  if (stat(path, &info) != 0) {
    return result;
  }

  result.success = true;
  result.bits = (uint32_t)(info.st_mode) & PERMISSION_MASK;
  return result;
}