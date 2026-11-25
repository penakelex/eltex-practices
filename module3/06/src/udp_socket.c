#define _POSIX_C_SOURCE 202405L

#include "udp_socket.h"

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int udp_socket_create() {
  return socket(AF_INET, SOCK_DGRAM, 0);
}

int udp_socket_create_broadcast() {
  const int fd = socket(AF_INET, SOCK_DGRAM, 0);

  if (fd < 0) {
    return -1;
  }

  const int enabled = 1;

  if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled)) < 0 ||
      setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

bool udp_socket_bind(const int fd, const char *address, const uint16_t port) {
  struct sockaddr_in local = {0};
  local.sin_family = AF_INET;
  local.sin_port = htons(port);
  local.sin_addr.s_addr = htonl(INADDR_ANY);

  if (address != nullptr) {
    if (inet_pton(AF_INET, address, &local.sin_addr) != 1) {
      return false;
    }
  }

  return bind(fd, (const struct sockaddr *)&local, sizeof(local)) == 0;
}

bool udp_socket_address(struct sockaddr_in *out, const char *address,
                        const uint16_t port) {
  if (out == nullptr || address == nullptr) {
    return false;
  }

  memset(out, 0, sizeof(*out));
  out->sin_family = AF_INET;
  out->sin_port = htons(port);

  return inet_pton(AF_INET, address, &out->sin_addr) == 1;
}

bool udp_socket_local_port(const int fd, uint16_t *out) {
  if (out == nullptr) {
    return false;
  }

  struct sockaddr_in local = {0};
  socklen_t length = sizeof(local);

  if (getsockname(fd, (struct sockaddr *)&local, &length) < 0) {
    return false;
  }

  *out = ntohs(local.sin_port);
  return true;
}

bool udp_socket_send_to(const int fd, const struct sockaddr_in *address,
                        const void *data, const size_t size) {
  if (address == nullptr || data == nullptr) {
    return false;
  }

  const ssize_t sent = sendto(
      fd, data, size, 0, (const struct sockaddr *)address, sizeof(*address));

  return sent == (ssize_t)size;
}

ssize_t udp_socket_receive(const int fd, void *buffer, const size_t capacity) {
  if (buffer == nullptr) {
    return -1;
  }

  return recv(fd, buffer, capacity, 0);
}
