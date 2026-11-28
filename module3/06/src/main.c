#define _POSIX_C_SOURCE 202405L

#include "udp_chat.h"

#include <stdlib.h>
#include <string.h>

#include "protocol.h"
#include "view.h"

static bool parse_port(const char *value, uint16_t *out) {
  char *end = nullptr;
  const long port = strtol(value, &end, 10);

  if (end == value || *end != '\0' || port <= 0 || port > 65535) {
    return false;
  }

  *out = (uint16_t)port;
  return true;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    view_usage(argv[0], "<port> <name>");
    return 1;
  }

  uint16_t port = 0;

  if (!parse_port(argv[1], &port)) {
    view_error("invalid port");
    return 1;
  }

  if (argv[2][0] == '\0' || strlen(argv[2]) >= CHAT_NAME_CAPACITY) {
    view_error("invalid name");
    return 1;
  }

  return udp_chat_run(argv[2], port);
}
