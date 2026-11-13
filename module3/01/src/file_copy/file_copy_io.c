#define _POSIX_C_SOURCE 202405L

#include "file_copy.h"
#include "file_copy_internal.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

bool file_copy_read_full(int fd, void *buffer, size_t size) {
  auto bytes = (char *)buffer;
  size_t received = 0;

  while (received < size) {
    const auto result = read(fd, bytes + received, size - received);

    if (result < 0) {
      if (errno == EINTR) {
        continue;
      }

      return false;
    }

    if (result == 0) {
      return false;
    }

    received += (size_t)result;
  }

  return true;
}

bool file_copy_write_full(int fd, const void *buffer, size_t size) {
  auto bytes = (const char *)buffer;
  size_t written = 0;

  while (written < size) {
    const auto result = write(fd, bytes + written, size - written);

    if (result < 0) {
      if (errno == EINTR) {
        continue;
      }

      return false;
    }

    written += (size_t)result;
  }

  return true;
}

bool file_copy_write_header(int fd, const char *name, uint64_t size) {
  uint32_t name_length = 0;

  if (name != nullptr && name[0] != '\0') {
    name_length = (uint32_t)strlen(name);
  }

  if (!file_copy_write_full(fd, &name_length, sizeof(name_length))) {
    return false;
  }

  if (name_length == 0) {
    return true;
  }

  if (!file_copy_write_full(fd, name, name_length)) {
    return false;
  }

  return file_copy_write_full(fd, &size, sizeof(size));
}

bool file_copy_read_header(int fd, char *name_out, size_t name_capacity,
                           uint32_t *name_length_out, uint64_t *size_out) {
  uint32_t name_length = 0;

  if (!file_copy_read_full(fd, &name_length, sizeof(name_length))) {
    return false;
  }

  *name_length_out = name_length;
  *size_out = 0;
  name_out[0] = '\0';

  if (name_length == 0) {
    return true;
  }

  if (name_length >= name_capacity) {
    return false;
  }

  if (!file_copy_read_full(fd, name_out, name_length)) {
    return false;
  }

  name_out[name_length] = '\0';

  return file_copy_read_full(fd, size_out, sizeof(*size_out));
}