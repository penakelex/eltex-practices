#pragma once

#include <stddef.h>
#include <stdint.h>

#include <sys/types.h>

int tcp_socket_create();
bool tcp_socket_set_reuse(const int fd);
bool tcp_socket_bind_listen(const int fd, const uint16_t port,
                            const int backlog);
int tcp_socket_accept(const int fd);
int tcp_socket_connect(const char *address, const uint16_t port);

bool tcp_socket_send_all(const int fd, const void *data, const size_t size);
ssize_t tcp_socket_receive_all(const int fd, void *buffer, const size_t size);
bool tcp_socket_receive_packet(const int fd, uint8_t *buffer,
                               const size_t capacity, size_t *out_size);
