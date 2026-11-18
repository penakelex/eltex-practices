#define _POSIX_C_SOURCE 202405L

#include "signal_state.h"

#include <string.h>

volatile sig_atomic_t signal_received = 0;

static void handle_signal(const int signal_number) {
  signal_received = signal_number;
}

void signal_state_install_handlers() {
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = handle_signal;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;

  sigaction(SIGINT, &action, nullptr);
  sigaction(SIGTERM, &action, nullptr);
}

void signal_state_reset() {
  signal_received = 0;
}
