#pragma once

#include <signal.h>
#include <stdint.h>

bool echo_server_run(int socket_descriptor, uint16_t port,
                     const volatile sig_atomic_t *stop_flag);
