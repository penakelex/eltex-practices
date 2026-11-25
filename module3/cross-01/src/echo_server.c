#define _POSIX_C_SOURCE 202405L

#include "echo_server.h"

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "client_table.h"
#include "echo_protocol.h"
#include "echo_view.h"
#include "raw_packet.h"

static constexpr size_t RECEIVE_CAPACITY = 65536;

static bool send_reply(const int socket_descriptor, const UdpPacket *request,
                       const uint8_t *payload, const size_t payload_length) {
  const UdpPacket reply = {
      .source_address = request->destination_address,
      .destination_address = request->source_address,
      .source_port = request->destination_port,
      .destination_port = request->source_port,
      .payload = payload,
      .payload_length = payload_length,
  };

  uint8_t buffer[RECEIVE_CAPACITY];
  const size_t length = raw_packet_build(buffer, sizeof(buffer), &reply);

  if (length == 0) {
    return false;
  }

  struct sockaddr_in destination = {0};
  destination.sin_family = AF_INET;
  destination.sin_addr.s_addr = request->source_address;
  destination.sin_port = htons(request->source_port);

  return sendto(socket_descriptor, buffer, length, 0,
                (const struct sockaddr *)&destination,
                sizeof(destination)) == (ssize_t)length;
}

static void handle_packet(const int socket_descriptor, ClientTable *table,
                          const uint16_t port, const UdpPacket *packet) {
  if (packet->destination_port != port) {
    return;
  }

  if (echo_protocol_is_close(packet->payload, packet->payload_length)) {
    client_table_reset(table, packet->source_address, packet->source_port);
    echo_view_show_client_reset(packet->source_address, packet->source_port);
    return;
  }

  uint32_t number = 0;

  if (!client_table_next_number(table, packet->source_address,
                                packet->source_port, &number)) {
    return;
  }

  echo_view_show_request(packet->source_address, packet->source_port,
                         packet->payload, packet->payload_length);

  uint8_t reply_payload[ECHO_MESSAGE_CAPACITY];
  const size_t text_length = packet->payload_length < ECHO_MESSAGE_CAPACITY - 16
                                 ? packet->payload_length
                                 : ECHO_MESSAGE_CAPACITY - 16;
  const int reply_length =
      snprintf((char *)reply_payload, sizeof(reply_payload), "%.*s %u",
               (int)text_length, (const char *)packet->payload, number);

  if (reply_length <= 0) {
    return;
  }

  send_reply(socket_descriptor, packet, reply_payload, (size_t)reply_length);
}

bool echo_server_run(const int socket_descriptor, const uint16_t port,
                     const volatile sig_atomic_t *stop_flag) {
  if (socket_descriptor < 0 || stop_flag == nullptr) {
    return false;
  }

  ClientTable table;
  client_table_initialize(&table);
  bool ok = true;

  while (*stop_flag == 0) {
    uint8_t buffer[RECEIVE_CAPACITY];
    const ssize_t received = recv(socket_descriptor, buffer, sizeof(buffer), 0);

    if (received < 0) {
      if (errno == EINTR) {
        continue;
      }

      echo_view_show_errno("recv");
      ok = false;
      break;
    }

    UdpPacket packet = {0};

    if (!raw_packet_parse(buffer, (size_t)received, &packet)) {
      continue;
    }

    handle_packet(socket_descriptor, &table, port, &packet);
  }

  client_table_free(&table);
  return ok;
}
