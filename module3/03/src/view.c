#define _POSIX_C_SOURCE 202405L

#include "view.h"

#include <stdio.h>

void view_usage(const char *program_name) {
  fprintf(stderr, "Usage: %s <queue_name>\n", program_name);
}

void view_invalid_name(const char *queue_name) {
  fprintf(stderr, "Invalid queue name: %s\n", queue_name);
}

void view_connected(const char *name) {
  fprintf(stderr, "Connected as %s. Type a message and press Enter;\n", name);
  fprintf(stderr, "Ctrl-C or Ctrl-D to quit.\n");
}

void view_no_owner(const char *queue_name) {
  fprintf(stderr, "No owner answered on %s, exiting\n", queue_name);
}

void view_peer_left(const char *name) {
  fprintf(stderr, "%s left the chat\n", name);
}

void view_owner_left() {
  fprintf(stderr, "Owner left the chat\n");
}

void view_message(const char *from, const char *text) {
  printf("%s> %s\n", from, text);
  fflush(stdout);
}
