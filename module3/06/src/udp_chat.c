#define _POSIX_C_SOURCE 202405L

#include "udp_chat.h"

#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "message.h"
#include "protocol.h"
#include "signal_state.h"
#include "udp_chat_internal.h"
#include "udp_socket.h"
#include "view.h"

static bool send_packet(UdpChatState *state, const ChatPacketType type,
                        const char *text) {
  ChatPacket packet = {
      .type = type, .file_size = 0, .chunk_data = nullptr, .chunk_size = 0};
  snprintf(packet.sender, CHAT_NAME_CAPACITY, "%s", state->name);

  if (text != nullptr) {
    snprintf(packet.text, CHAT_TEXT_CAPACITY, "%s", text);
  }

  uint8_t buffer[CHAT_PACKET_HEADER_SIZE + CHAT_NAME_CAPACITY + 1 +
                 CHAT_TEXT_CAPACITY];
  size_t size = 0;

  if (!chat_packet_serialize(&packet, buffer, sizeof(buffer), &size)) {
    return false;
  }

  return udp_socket_send_to(state->fd, &state->broadcast_address, buffer, size);
}

static void handle_incoming(UdpChatState *state) {
  uint8_t buffer[CHAT_PACKET_HEADER_SIZE + CHAT_NAME_CAPACITY + 1 +
                 CHAT_TEXT_CAPACITY];
  const ssize_t received =
      udp_socket_receive(state->fd, buffer, sizeof(buffer));

  if (received <= 0) {
    return;
  }

  ChatPacket packet = {0};

  if (!chat_packet_deserialize(buffer, (size_t)received, &packet)) {
    return;
  }

  if (strncmp(packet.sender, state->name, CHAT_NAME_CAPACITY) == 0) {
    return;
  }

  switch (packet.type) {
  case CHAT_PACKET_JOIN: {
    view_joined(packet.sender);
    break;
  }
  case CHAT_PACKET_LEAVE: {
    view_left(packet.sender);
    break;
  }
  case CHAT_PACKET_TEXT: {
    view_message(packet.sender, packet.text);
    break;
  }
  default: {
    break;
  }
  }
}

static bool handle_input(UdpChatState *state) {
  char line[CHAT_TEXT_CAPACITY * 2];

  if (fgets(line, sizeof(line), stdin) == nullptr) {
    return false;
  }

  ChatInput input = {0};

  if (!chat_input_parse(line, &input)) {
    return true;
  }

  switch (input.type) {
  case CHAT_INPUT_TEXT: {
    send_packet(state, CHAT_PACKET_TEXT, input.text);
    break;
  }
  case CHAT_INPUT_FILE: {
    view_error("file transfer is not supported in udp chat");
    break;
  }
  case CHAT_INPUT_UNKNOWN_COMMAND: {
    view_error("unknown command");
    break;
  }
  case CHAT_INPUT_EMPTY: {
    break;
  }
  }

  return true;
}

int udp_chat_loop(UdpChatState *state) {
  struct pollfd descriptors[2] = {{.fd = STDIN_FILENO, .events = POLLIN},
                                  {.fd = state->fd, .events = POLLIN}};

  while (signal_received == 0) {
    const int ready = poll(descriptors, 2, 200);

    if (ready < 0) {
      break;
    }

    if ((descriptors[1].revents & POLLIN) != 0) {
      handle_incoming(state);
    }

    if ((descriptors[0].revents & (POLLIN | POLLHUP)) != 0) {
      if (!handle_input(state)) {
        break;
      }
    }
  }

  return 0;
}

int udp_chat_run(const char *name, const uint16_t port) {
  signal_state_install_handlers();

  UdpChatState state = {.fd = -1, .port = port};
  snprintf(state.name, CHAT_NAME_CAPACITY, "%s", name);

  state.fd = udp_socket_create_broadcast();

  if (state.fd < 0) {
    view_error("failed to create udp socket");
    return 1;
  }

  if (!udp_socket_bind(state.fd, nullptr, port)) {
    view_error("failed to bind udp socket");
    close(state.fd);
    return 1;
  }

  if (!udp_socket_address(&state.broadcast_address, "255.255.255.255", port)) {
    view_error("failed to prepare broadcast address");
    close(state.fd);
    return 1;
  }

  view_connected(state.name);
  send_packet(&state, CHAT_PACKET_JOIN, nullptr);
  udp_chat_loop(&state);
  send_packet(&state, CHAT_PACKET_LEAVE, nullptr);
  close(state.fd);
  return 0;
}
