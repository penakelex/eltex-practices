#define _POSIX_C_SOURCE 202405L

#include "broker.h"
#include "publisher.h"
#include "subscriber.h"
#include "view.h"

#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    view_usage(argv[0]);
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "-b") == 0) {
    if (argc != 2) {
      view_usage(argv[0]);
      return EXIT_FAILURE;
    }

    if (!broker_run()) {
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  if (strcmp(argv[1], "-p") == 0) {
    if (argc != 3) {
      view_usage(argv[0]);
      return EXIT_FAILURE;
    }

    if (!publisher_run(argv[2])) {
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  if (strcmp(argv[1], "-s") == 0) {
    if (argc < 3) {
      view_usage(argv[0]);
      return EXIT_FAILURE;
    }

    if (!subscriber_run(&argv[2], argc - 2)) {
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  view_usage(argv[0]);
  return EXIT_FAILURE;
}