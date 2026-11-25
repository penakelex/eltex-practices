#define _POSIX_C_SOURCE 202405L

#include "view.h"

#include <stdio.h>

void view_usage(const char *program_name, const char *hint) {
  fprintf(stderr, "Usage: %s %s\n", program_name, hint);
}

void view_error(const char *message) {
  fprintf(stderr, "Error: %s\n", message);
}

void view_connected(const char *name) {
  fprintf(stderr, "Connected as %s. Type a message and press Enter;\n", name);
  fprintf(stderr, "use /file <path> to send a file, Ctrl-C to quit.\n");
}

void view_joined(const char *name) {
  fprintf(stderr, "%s joined the chat\n", name);
}

void view_left(const char *name) {
  fprintf(stderr, "%s left the chat\n", name);
}

void view_message(const char *sender, const char *text) {
  printf("%s> %s\n", sender, text);
  fflush(stdout);
}

void view_file_incoming(const char *sender, const char *file_name) {
  fprintf(stderr, "%s is sending a file: %s\n", sender, file_name);
}

void view_file_saved(const char *path) {
  fprintf(stderr, "File saved as %s\n", path);
}
