#define _POSIX_C_SOURCE 202405L

#include "file_transfer.h"

#include <stdio.h>
#include <string.h>

#include "tcp_socket.h"

static bool send_simple_packet(const int fd, const ChatPacketType type,
                               const char *sender, const char *file_name,
                               const uint64_t file_size,
                               const uint8_t *chunk_data,
                               const size_t chunk_size) {
  ChatPacket packet = {.type = type,
                       .file_size = file_size,
                       .chunk_data = chunk_data,
                       .chunk_size = chunk_size};
  snprintf(packet.sender, CHAT_NAME_CAPACITY, "%s", sender);

  if (file_name != nullptr) {
    snprintf(packet.file_name, CHAT_FILE_NAME_CAPACITY, "%s", file_name);
  }

  uint8_t buffer[CHAT_PACKET_BUFFER_SIZE];
  size_t size = 0;

  if (!chat_packet_serialize(&packet, buffer, sizeof(buffer), &size)) {
    return false;
  }

  return tcp_socket_send_all(fd, buffer, size);
}

static const char *base_name(const char *path) {
  const char *slash = strrchr(path, '/');

  if (slash == nullptr) {
    return path;
  }

  return slash + 1;
}

bool file_transfer_send(const int fd, const char *sender, const char *path) {
  if (sender == nullptr || path == nullptr) {
    return false;
  }

  FILE *file = fopen(path, "rb");

  if (file == nullptr) {
    return false;
  }

  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    return false;
  }

  const long length = ftell(file);

  if (length < 0 || fseek(file, 0, SEEK_SET) != 0) {
    fclose(file);
    return false;
  }

  const char *name = base_name(path);

  if (strlen(name) >= CHAT_FILE_NAME_CAPACITY) {
    fclose(file);
    return false;
  }

  bool ok = send_simple_packet(fd, CHAT_PACKET_FILE_META, sender, name,
                               (uint64_t)length, nullptr, 0);

  uint8_t chunk[CHAT_CHUNK_SIZE];

  while (ok) {
    const size_t count = fread(chunk, 1, sizeof(chunk), file);

    if (count == 0) {
      break;
    }

    ok = send_simple_packet(fd, CHAT_PACKET_FILE_CHUNK, sender, nullptr, 0,
                            chunk, count);
  }

  if (ok && ferror(file)) {
    ok = false;
  }

  if (ok) {
    ok = send_simple_packet(fd, CHAT_PACKET_FILE_END, sender, nullptr, 0,
                            nullptr, 0);
  }

  fclose(file);
  return ok;
}

void file_receiver_initialize(FileReceiver *receiver) {
  if (receiver == nullptr) {
    return;
  }

  receiver->file = nullptr;
  receiver->path[0] = '\0';
  receiver->expected = 0;
  receiver->received = 0;
}

void file_receiver_free(FileReceiver *receiver) {
  if (receiver == nullptr) {
    return;
  }

  if (receiver->file != nullptr) {
    fclose(receiver->file);
  }

  file_receiver_initialize(receiver);
}

bool file_receiver_begin(FileReceiver *receiver, const char *file_name,
                         const uint64_t file_size) {
  if (receiver == nullptr || file_name == nullptr) {
    return false;
  }

  file_receiver_free(receiver);

  const int written = snprintf(receiver->path, sizeof(receiver->path),
                               "%s.received", file_name);

  if (written < 0 || (size_t)written >= sizeof(receiver->path)) {
    return false;
  }

  receiver->file = fopen(receiver->path, "wb");

  if (receiver->file == nullptr) {
    return false;
  }

  receiver->expected = file_size;
  receiver->received = 0;
  return true;
}

bool file_receiver_append(FileReceiver *receiver, const uint8_t *data,
                          const size_t size) {
  if (receiver == nullptr || receiver->file == nullptr) {
    return false;
  }

  if (size > 0 && fwrite(data, 1, size, receiver->file) != size) {
    return false;
  }

  receiver->received += size;
  return true;
}

bool file_receiver_finish(FileReceiver *receiver) {
  if (receiver == nullptr || receiver->file == nullptr) {
    return false;
  }

  const bool complete = receiver->received == receiver->expected;

  fclose(receiver->file);
  receiver->file = nullptr;
  return complete;
}
