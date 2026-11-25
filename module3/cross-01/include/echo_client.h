#pragma once

#include <signal.h>
#include <stdint.h>

bool echo_client_run(int socket_descriptor, uint32_t server_address,
                     uint16_t port, const volatile sig_atomic_t *stop_flag);
