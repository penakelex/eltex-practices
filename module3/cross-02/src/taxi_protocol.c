#define _POSIX_C_SOURCE 202405L

#include "taxi_protocol.h"

#include <unistd.h>

static bool write_all(const int fd, const void *data, size_t size) {
  const uint8_t *bytes = data;

  while (size > 0) {
    const ssize_t written = write(fd, bytes, size);

    if (written <= 0) {
      return false;
    }

    bytes += written;
    size -= (size_t)written;
  }

  return true;
}

static bool read_all(const int fd, void *data, size_t size) {
  uint8_t *bytes = data;

  while (size > 0) {
    const ssize_t received = read(fd, bytes, size);

    if (received <= 0) {
      return false;
    }

    bytes += received;
    size -= (size_t)received;
  }

  return true;
}

bool taxi_protocol_write_command(const int fd,
                                 const TaxiCommandMessage *message) {
  if (fd < 0 || message == nullptr) {
    return false;
  }

  return write_all(fd, message, sizeof(*message));
}

bool taxi_protocol_read_command(const int fd, TaxiCommandMessage *out) {
  if (fd < 0 || out == nullptr) {
    return false;
  }

  return read_all(fd, out, sizeof(*out));
}

bool taxi_protocol_write_event(const int fd, const TaxiEventMessage *message) {
  if (fd < 0 || message == nullptr) {
    return false;
  }

  return write_all(fd, message, sizeof(*message));
}

bool taxi_protocol_read_event(const int fd, TaxiEventMessage *out) {
  if (fd < 0 || out == nullptr) {
    return false;
  }

  return read_all(fd, out, sizeof(*out));
}
