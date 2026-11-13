#pragma once

#include <signal.h>

extern volatile sig_atomic_t signal_received;

void signal_state_install_handlers();
void signal_state_reset();