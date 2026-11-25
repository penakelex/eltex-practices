#define _POSIX_C_SOURCE 202405L

#include "echo_client.h"

#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "echo_protocol.h"
#include "echo_view.h"
#include "raw_packet.h"

static constexpr size_t RECEIVE_CAPACITY = 65536;
static constexpr uint16_t SOURCE_PORT_BASE = 40000;
static constexpr uint16_t SOURCE_PORT_RANGE = 20000;

static bool resolve_local_address(const uint32_t server_address,
                                  const uint16_t port, uint32_t *out_address) {
  const int probe = socket(AF_INET, SOCK_DGRAM, 0);

  if (probe < 0) {
    return false;
  }

  struct sockaddr_in server = {0};
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = server_address;

  struct sockaddr_in local = {0};
  socklen_t local_length = sizeof(local);

  bool ok =
      connect(probe, (const struct sockaddr *)&server, sizeof(server)) == 0;

  if (ok) {
    ok = getsockname(probe, (struct sockaddr *)&local, &local_length) == 0;
  }

  if (ok) {
    *out_address = local.sin_addr.s_addr;
  }

  close(probe);
  return ok;
}

static bool send_payload(const int socket_descriptor, const uint32_t local,
                         const uint32_t server, const uint16_t local_port,
                         const uint16_t server_port, const uint8_t *payload,
                         const size_t payload_length) {
  const UdpPacket packet = {
      .source_address = local,
      .destination_address = server,
      .source_port = local_port,
      .destination_port = server_port,
      .payload = payload,
      .payload_length = payload_length,
  };

  uint8_t buffer[RECEIVE_CAPACITY];
  const size_t length = raw_packet_build(buffer, sizeof(buffer), &packet);

  if (length == 0) {
    return false;
  }

  struct sockaddr_in destination = {0};
  destination.sin_family = AF_INET;
  destination.sin_addr.s_addr = server;
  destination.sin_port = htons(server_port);

  return sendto(socket_descriptor, buffer, length, 0,
                (const struct sockaddr *)&destination,
                sizeof(destination)) == (ssize_t)length;
}

static void handle_reply(const int socket_descriptor, const uint16_t port) {
  uint8_t buffer[RECEIVE_CAPACITY];
  const ssize_t received = recv(socket_descriptor, buffer, sizeof(buffer), 0);

  if (received < 0) {
    return;
  }

  UdpPacket packet = {0};

  if (!raw_packet_parse(buffer, (size_t)received, &packet)) {
    return;
  }

  if (packet.destination_port != port) {
    return;
  }

  echo_view_show_reply(packet.payload, packet.payload_length);
}

static bool read_line(char *buffer, const size_t capacity, size_t *out_length) {
  if (fgets(buffer, (int)capacity, stdin) == nullptr) {
    return false;
  }

  size_t length = strlen(buffer);

  while (length > 0 &&
         (buffer[length - 1] == '\n' || buffer[length - 1] == '\r')) {
    buffer[--length] = '\0';
  }

  *out_length = length;
  return true;
}

bool echo_client_run(const int socket_descriptor, const uint32_t server_address,
                     const uint16_t port,
                     const volatile sig_atomic_t *stop_flag) {
  if (socket_descriptor < 0 || stop_flag == nullptr) {
    return false;
  }

  uint32_t local_address = 0;

  if (!resolve_local_address(server_address, port, &local_address)) {
    echo_view_show_errno("определение локального адреса");
    return false;
  }

  const auto local_port =
      (uint16_t)(SOURCE_PORT_BASE + getpid() % SOURCE_PORT_RANGE);
  bool ok = true;

  while (*stop_flag == 0) {
    struct pollfd descriptors[2] = {
        {.fd = STDIN_FILENO, .events = POLLIN},
        {.fd = socket_descriptor, .events = POLLIN},
    };

    const int ready = poll(descriptors, 2, -1);

    if (ready < 0) {
      if (errno == EINTR) {
        continue;
      }

      echo_view_show_errno("poll");
      ok = false;
      break;
    }

    if ((descriptors[1].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
      echo_view_show_error("raw-сокет перешёл в состояние ошибки");
      ok = false;
      break;
    }

    if ((descriptors[1].revents & POLLIN) != 0) {
      handle_reply(socket_descriptor, local_port);
    }

    if ((descriptors[0].revents & (POLLERR | POLLHUP)) != 0) {
      break;
    }

    if ((descriptors[0].revents & POLLIN) != 0) {
      char message[ECHO_MESSAGE_CAPACITY];
      size_t message_length = 0;

      if (!read_line(message, sizeof(message), &message_length)) {
        break;
      }

      if (message_length == 0) {
        continue;
      }

      if (!send_payload(socket_descriptor, local_address, server_address,
                        local_port, port, (const uint8_t *)message,
                        message_length)) {
        echo_view_show_errno("sendto");
        ok = false;
        break;
      }
    }
  }

  uint8_t close_payload[8];
  const size_t close_length =
      echo_protocol_build_close(close_payload, sizeof(close_payload));

  if (close_length > 0) {
    send_payload(socket_descriptor, local_address, server_address, local_port,
                 port, close_payload, close_length);
  }

  return ok;
}
