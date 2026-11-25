#define _POSIX_C_SOURCE 202405L

#include "tcp_socket.h"

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "protocol.h"

int tcp_socket_create() {
  return socket(AF_INET, SOCK_STREAM, 0);
}

bool tcp_socket_set_reuse(const int fd) {
  const int enabled = 1;
  return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) ==
         0;
}

bool tcp_socket_bind_listen(const int fd, const uint16_t port,
                            const int backlog) {
  struct sockaddr_in local = {0};
  local.sin_family = AF_INET;
  local.sin_port = htons(port);
  local.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(fd, (const struct sockaddr *)&local, sizeof(local)) < 0) {
    return false;
  }

  return listen(fd, backlog) == 0;
}

int tcp_socket_accept(const int fd) {
  return accept(fd, nullptr, nullptr);
}

int tcp_socket_connect(const char *address, const uint16_t port) {
  const int fd = tcp_socket_create();

  if (fd < 0) {
    return -1;
  }

  struct sockaddr_in remote = {0};
  remote.sin_family = AF_INET;
  remote.sin_port = htons(port);

  if (inet_pton(AF_INET, address, &remote.sin_addr) != 1) {
    close(fd);
    return -1;
  }

  if (connect(fd, (const struct sockaddr *)&remote, sizeof(remote)) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

bool tcp_socket_send_all(const int fd, const void *data, const size_t size) {
  if (data == nullptr) {
    return false;
  }

  size_t sent = 0;

  while (sent < size) {
    const ssize_t count =
        send(fd, (const uint8_t *)data + sent, size - sent, MSG_NOSIGNAL);

    if (count <= 0) {
      return false;
    }

    sent += (size_t)count;
  }

  return true;
}

ssize_t tcp_socket_receive_all(const int fd, void *buffer, const size_t size) {
  if (buffer == nullptr) {
    return -1;
  }

  size_t received = 0;

  while (received < size) {
    const ssize_t count =
        recv(fd, (uint8_t *)buffer + received, size - received, 0);

    if (count < 0) {
      return -1;
    }

    if (count == 0) {
      return 0;
    }

    received += (size_t)count;
  }

  return (ssize_t)received;
}

bool tcp_socket_receive_packet(const int fd, uint8_t *buffer,
                               const size_t capacity, size_t *out_size) {
  if (buffer == nullptr || out_size == nullptr) {
    return false;
  }

  if (capacity < CHAT_PACKET_HEADER_SIZE) {
    return false;
  }

  if (tcp_socket_receive_all(fd, buffer, CHAT_PACKET_HEADER_SIZE) <= 0) {
    return false;
  }

  ChatPacketType type = 0;
  size_t payload_size = 0;

  if (!chat_packet_parse_header(buffer, CHAT_PACKET_HEADER_SIZE, &type,
                                &payload_size)) {
    return false;
  }

  if (CHAT_PACKET_HEADER_SIZE + payload_size > capacity) {
    return false;
  }

  if (payload_size > 0 &&
      tcp_socket_receive_all(fd, buffer + CHAT_PACKET_HEADER_SIZE,
                             payload_size) <= 0) {
    return false;
  }

  *out_size = CHAT_PACKET_HEADER_SIZE + payload_size;
  return true;
}
