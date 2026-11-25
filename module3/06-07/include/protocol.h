#pragma once

#include <stddef.h>
#include <stdint.h>

[[maybe_unused]] constexpr size_t CHAT_NAME_CAPACITY = 32;
[[maybe_unused]] constexpr size_t CHAT_TEXT_CAPACITY = 1024;
[[maybe_unused]] constexpr size_t CHAT_FILE_NAME_CAPACITY = 256;
[[maybe_unused]] constexpr size_t CHAT_CHUNK_SIZE = 4096;
[[maybe_unused]] constexpr size_t CHAT_PACKET_HEADER_SIZE = 8;
[[maybe_unused]] constexpr size_t CHAT_PACKET_BUFFER_SIZE =
    CHAT_PACKET_HEADER_SIZE + CHAT_NAME_CAPACITY + 1 + CHAT_CHUNK_SIZE;

typedef enum : uint32_t {
  CHAT_PACKET_JOIN = 1,
  CHAT_PACKET_LEAVE = 2,
  CHAT_PACKET_TEXT = 3,
  CHAT_PACKET_FILE_META = 4,
  CHAT_PACKET_FILE_CHUNK = 5,
  CHAT_PACKET_FILE_END = 6
} ChatPacketType;

typedef struct {
  ChatPacketType type;
  char sender[CHAT_NAME_CAPACITY];
  char text[CHAT_TEXT_CAPACITY];
  char file_name[CHAT_FILE_NAME_CAPACITY];
  uint64_t file_size;
  const uint8_t *chunk_data;
  size_t chunk_size;
} ChatPacket;

bool chat_packet_serialize(const ChatPacket *packet, uint8_t *buffer,
                           const size_t capacity, size_t *out_size);

bool chat_packet_deserialize(const uint8_t *buffer, const size_t size,
                             ChatPacket *out);

bool chat_packet_parse_header(const uint8_t *buffer, const size_t size,
                              ChatPacketType *out_type,
                              size_t *out_payload_size);
