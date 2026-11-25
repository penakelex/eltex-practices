#pragma once

#include <stddef.h>
#include <stdint.h>

#include <netinet/in.h>

int udp_socket_create();
int udp_socket_create_broadcast();

bool udp_socket_bind(const int fd, const char *address, const uint16_t port);
bool udp_socket_address(struct sockaddr_in *out, const char *address,
                        const uint16_t port);
bool udp_socket_local_port(const int fd, uint16_t *out);
bool udp_socket_send_to(const int fd, const struct sockaddr_in *address,
                        const void *data, const size_t size);
ssize_t udp_socket_receive(const int fd, void *buffer, const size_t capacity);
