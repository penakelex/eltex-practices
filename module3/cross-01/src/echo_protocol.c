#define _POSIX_C_SOURCE 202405L

#include "echo_protocol.h"

#include <string.h>

static const uint8_t CLOSE_MARKER[] = {0x01, 'C', 'L', 'O', 'S', 'E'};

bool echo_protocol_is_close(const uint8_t *payload, size_t length) {
  if (payload == nullptr) {
    return false;
  }

  return length == sizeof(CLOSE_MARKER) &&
         memcmp(payload, CLOSE_MARKER, sizeof(CLOSE_MARKER)) == 0;
}

size_t echo_protocol_build_close(uint8_t *buffer, size_t capacity) {
  if (buffer == nullptr || capacity < sizeof(CLOSE_MARKER)) {
    return 0;
  }

  memcpy(buffer, CLOSE_MARKER, sizeof(CLOSE_MARKER));
  return sizeof(CLOSE_MARKER);
}
