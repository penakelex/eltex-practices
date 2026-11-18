#define _POSIX_C_SOURCE 202405L

#include "ipc_posix.h"
#include "consumer.h"
#include "producer.h"
#include "view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

[[maybe_unused]] const char *const DEFAULT_OBJECT_NAME = "/eltex_practice05";

static bool is_valid_object_name(const char *name) {
  return name != nullptr && name[0] == '/' && name[1] != '\0' &&
         strchr(name + 1, '/') == nullptr;
}

static bool parse_unsigned(const char *text, unsigned long *value_out) {
  if (text == nullptr || text[0] == '\0') {
    return false;
  }

  char *end = nullptr;
  const auto parsed = strtoul(text, &end, 10);

  if (end == text || *end != '\0') {
    return false;
  }

  *value_out = parsed;
  return true;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    view_usage(argv[0]);
    return EXIT_FAILURE;
  }

  char object_name[128] = {0};

  if (strcmp(argv[2], "default") == 0) {
    snprintf(object_name, sizeof(object_name), "%s", DEFAULT_OBJECT_NAME);
  } else {
    snprintf(object_name, sizeof(object_name), "%s", argv[2]);
  }

  if (!is_valid_object_name(object_name)) {
    view_error("invalid object name: must look like /name without extra '/'");
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "-p") == 0) {
    if (argc < 4) {
      view_usage(argv[0]);
      return EXIT_FAILURE;
    }

    unsigned long capacity = 0;

    if (!parse_unsigned(argv[3], &capacity) || capacity == 0 ||
        capacity > 1048576) {
      view_error("invalid capacity");
      return EXIT_FAILURE;
    }

    unsigned long produce_delay = 1;
    unsigned long poll_delay = 1;

    if (argc > 4 && !parse_unsigned(argv[4], &produce_delay)) {
      view_error("invalid produce delay");
      return EXIT_FAILURE;
    }

    if (argc > 5 && !parse_unsigned(argv[5], &poll_delay)) {
      view_error("invalid poll delay");
      return EXIT_FAILURE;
    }

    if (!producer_run(ipc_posix_ops(), object_name, (size_t)capacity,
                      (unsigned int)produce_delay, (unsigned int)poll_delay)) {
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  if (strcmp(argv[1], "-c") == 0) {
    unsigned long poll_delay = 1;

    if (argc > 3 && !parse_unsigned(argv[3], &poll_delay)) {
      view_error("invalid poll delay");
      return EXIT_FAILURE;
    }

    if (!consumer_run(ipc_posix_ops(), object_name, (unsigned int)poll_delay)) {
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  view_usage(argv[0]);
  return EXIT_FAILURE;
}
