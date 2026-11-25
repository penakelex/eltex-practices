#define _POSIX_C_SOURCE 202405L

#include "chat_client.h"

#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "file_transfer.h"
#include "message.h"
#include "protocol.h"
#include "signal_state.h"
#include "tcp_socket.h"
#include "view.h"

constexpr int CHAT_CLIENT_POLL_TIMEOUT_MS = 200;

typedef struct {
  int fd;
  char name[CHAT_NAME_CAPACITY];
  FileReceiver receiver;
} ChatClientState;

static bool send_packet(ChatClientState *state, const ChatPacketType type,
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

  return tcp_socket_send_all(state->fd, buffer, size);
}

static void handle_incoming(ChatClientState *state) {
  uint8_t buffer[CHAT_PACKET_BUFFER_SIZE];
  size_t size = 0;

  if (!tcp_socket_receive_packet(state->fd, buffer, sizeof(buffer), &size)) {
    view_error("connection to server lost");
    signal_received = SIGTERM;
    return;
  }

  ChatPacket packet = {0};

  if (!chat_packet_deserialize(buffer, size, &packet)) {
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
  case CHAT_PACKET_FILE_META: {
    view_file_incoming(packet.sender, packet.file_name);

    if (!file_receiver_begin(&state->receiver, packet.file_name,
                             packet.file_size)) {
      view_error("failed to open file for writing");
    }

    break;
  }
  case CHAT_PACKET_FILE_CHUNK: {
    file_receiver_append(&state->receiver, packet.chunk_data,
                         packet.chunk_size);
    break;
  }
  case CHAT_PACKET_FILE_END: {
    if (file_receiver_finish(&state->receiver)) {
      view_file_saved(state->receiver.path);
    } else {
      view_error("received file is incomplete");
    }

    break;
  }
  default: {
    break;
  }
  }
}

static bool handle_input(ChatClientState *state) {
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
    if (!file_transfer_send(state->fd, state->name, input.file_path)) {
      view_error("failed to send file");
    }

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

int chat_client_run(const char *address, const uint16_t port,
                    const char *name) {
  signal_state_install_handlers();

  ChatClientState state = {.fd = -1};
  snprintf(state.name, CHAT_NAME_CAPACITY, "%s", name);
  file_receiver_initialize(&state.receiver);

  state.fd = tcp_socket_connect(address, port);

  if (state.fd < 0) {
    view_error("failed to connect to server");
    return 1;
  }

  view_connected(state.name);
  send_packet(&state, CHAT_PACKET_JOIN, nullptr);

  struct pollfd descriptors[2] = {{.fd = STDIN_FILENO, .events = POLLIN},
                                  {.fd = state.fd, .events = POLLIN}};

  while (signal_received == 0) {
    const int ready = poll(descriptors, 2, CHAT_CLIENT_POLL_TIMEOUT_MS);

    if (ready < 0) {
      break;
    }

    if ((descriptors[1].revents & (POLLIN | POLLHUP | POLLERR)) != 0) {
      handle_incoming(&state);
    }

    if (signal_received != 0) {
      break;
    }

    if ((descriptors[0].revents & (POLLIN | POLLHUP)) != 0) {
      if (!handle_input(&state)) {
        break;
      }
    }
  }

  send_packet(&state, CHAT_PACKET_LEAVE, nullptr);
  file_receiver_free(&state.receiver);
  close(state.fd);
  return 0;
}
