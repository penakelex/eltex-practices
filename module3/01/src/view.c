#define _POSIX_C_SOURCE 202405L

#include "view.h"

#include <stdio.h>
#include <string.h>

void file_copy_view_usage(const char *program_name) {
  fprintf(stderr, "Usage: %s [-p fifo_base_name] file1 [file2 ...]\n",
          program_name);
}

void file_copy_view_open_error(const char *file_name, int error_code) {
  fprintf(stderr, "Cannot open '%s': %s\n", file_name, strerror(error_code));
}

void file_copy_view_stat_error(const char *file_name, int error_code) {
  fprintf(stderr, "Cannot stat '%s': %s\n", file_name, strerror(error_code));
}

void file_copy_view_not_regular_file(const char *file_name) {
  fprintf(stderr, "'%s' is not a regular file\n", file_name);
}

void file_copy_view_name_too_long(const char *file_name) {
  fprintf(stderr, "File name too long: '%s'\n", file_name);
}

void file_copy_view_create_error(const char *file_name, int error_code) {
  fprintf(stderr, "Cannot create copy of '%s': %s\n", file_name,
          strerror(error_code));
}