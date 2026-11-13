#define _POSIX_C_SOURCE 202405L

#include "file_copy.h"
#include "view.h"

#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  FileCopyOptions options = {
      .use_named_pipe = false,
      .fifo_base_name = nullptr,
      .file_count = 0,
      .file_names = nullptr,
  };

  int arg_index = 1;

  if (arg_index < argc && strcmp(argv[arg_index], "-p") == 0) {
    if (arg_index + 1 >= argc) {
      file_copy_view_usage(argv[0]);
      return EXIT_FAILURE;
    }

    options.use_named_pipe = true;
    options.fifo_base_name = argv[arg_index + 1];
    arg_index += 2;
  }

  if (arg_index >= argc) {
    file_copy_view_usage(argv[0]);
    return EXIT_FAILURE;
  }

  options.file_names = &argv[arg_index];
  options.file_count = argc - arg_index;

  if (!file_copy_run(&options)) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}