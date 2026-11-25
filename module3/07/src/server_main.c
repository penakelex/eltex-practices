#define _POSIX_C_SOURCE 202405L

#include "chat_server.h"

#include <stdlib.h>

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
  if (argc != 2) {
    view_usage(argv[0], "<port>");
    return 1;
  }

  uint16_t port = 0;

  if (!parse_port(argv[1], &port)) {
    view_error("invalid port");
    return 1;
  }

  return chat_server_run(port);
}
