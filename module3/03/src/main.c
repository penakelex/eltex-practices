#define _POSIX_C_SOURCE 202405L

#include "chat.h"
#include "view.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

static bool valid_queue_name(const char *name) {
  if (name[0] == '\0' || name[0] == '/') {
    return false;
  }

  for (const char *c = name; *c != '\0'; ++c) {
    if (!isalnum((unsigned char)*c) && *c != '_' && *c != '-') {
      return false;
    }
  }

  return true;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    view_usage(argv[0]);
    return EXIT_FAILURE;
  }

  if (!valid_queue_name(argv[1])) {
    view_invalid_name(argv[1]);
    view_usage(argv[0]);
    return EXIT_FAILURE;
  }

  char base_name[256];
  snprintf(base_name, sizeof(base_name), "/%s", argv[1]);

  if (!chat_run(base_name)) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
