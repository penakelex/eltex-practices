#define _POSIX_C_SOURCE 202405L

#include "consumer.h"
#include "producer.h"
#include "view.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool ensure_key_file(const char *key_path) {
  const int fd = open(key_path, O_WRONLY | O_CREAT, 0600);

  if (fd == -1) {
    return false;
  }

  close(fd);
  return true;
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

  const char *key_path = argv[2];

  if (!ensure_key_file(key_path)) {
    view_error("cannot create key file");
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

    if (!producer_run(key_path, (size_t)capacity, (unsigned int)produce_delay,
                      (unsigned int)poll_delay)) {
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

    if (!consumer_run(key_path, (unsigned int)poll_delay)) {
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  view_usage(argv[0]);
  return EXIT_FAILURE;
}
