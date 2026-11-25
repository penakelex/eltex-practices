#define _POSIX_C_SOURCE 202405L

#include "protocol.h"

#include <string.h>

static void write_header(uint8_t *buffer, const ChatPacketType type,
                         const size_t payload_size) {
  const uint32_t raw_type = (uint32_t)type;
  const uint32_t raw_size = (uint32_t)payload_size;
  memcpy(buffer, &raw_type, sizeof(raw_type));
  memcpy(buffer + sizeof(raw_type), &raw_size, sizeof(raw_size));
}

static bool write_string(uint8_t *buffer, const size_t capacity, size_t *offset,
                         const char *value) {
  const size_t length = strlen(value) + 1;

  if (*offset + length > capacity) {
    return false;
  }

  memcpy(buffer + *offset, value, length);
  *offset += length;
  return true;
}

bool chat_packet_serialize(const ChatPacket *packet, uint8_t *buffer,
                           const size_t capacity, size_t *out_size) {
  if (packet == nullptr || buffer == nullptr || out_size == nullptr) {
    return false;
  }

  if (capacity < CHAT_PACKET_HEADER_SIZE) {
    return false;
  }

  size_t offset = CHAT_PACKET_HEADER_SIZE;

  if (!write_string(buffer, capacity, &offset, packet->sender)) {
    return false;
  }

  switch (packet->type) {
  case CHAT_PACKET_JOIN:
  case CHAT_PACKET_LEAVE:
  case CHAT_PACKET_FILE_END: {
    break;
  }
  case CHAT_PACKET_TEXT: {
    if (!write_string(buffer, capacity, &offset, packet->text)) {
      return false;
    }
    break;
  }
  case CHAT_PACKET_FILE_META: {
    if (!write_string(buffer, capacity, &offset, packet->file_name)) {
      return false;
    }

    if (offset + sizeof(packet->file_size) > capacity) {
      return false;
    }

    memcpy(buffer + offset, &packet->file_size, sizeof(packet->file_size));
    offset += sizeof(packet->file_size);
    break;
  }
  case CHAT_PACKET_FILE_CHUNK: {
    if (packet->chunk_data == nullptr && packet->chunk_size > 0) {
      return false;
    }

    if (offset + packet->chunk_size > capacity) {
      return false;
    }

    memcpy(buffer + offset, packet->chunk_data, packet->chunk_size);
    offset += packet->chunk_size;
    break;
  }
  default: {
    return false;
  }
  }

  write_header(buffer, packet->type, offset - CHAT_PACKET_HEADER_SIZE);
  *out_size = offset;
  return true;
}

bool chat_packet_parse_header(const uint8_t *buffer, const size_t size,
                              ChatPacketType *out_type,
                              size_t *out_payload_size) {
  if (buffer == nullptr || out_type == nullptr || out_payload_size == nullptr) {
    return false;
  }

  if (size < CHAT_PACKET_HEADER_SIZE) {
    return false;
  }

  uint32_t raw_type = 0;
  uint32_t raw_size = 0;
  memcpy(&raw_type, buffer, sizeof(raw_type));
  memcpy(&raw_size, buffer + sizeof(raw_type), sizeof(raw_size));

  *out_type = (ChatPacketType)raw_type;
  *out_payload_size = raw_size;
  return true;
}

static bool read_string(const uint8_t *buffer, const size_t size,
                        size_t *offset, char *out, const size_t capacity) {
  const size_t remaining = size - *offset;
  const uint8_t *cursor = buffer + *offset;
  const uint8_t *terminator = memchr(cursor, '\0', remaining);

  if (terminator == nullptr) {
    return false;
  }

  const size_t length = (size_t)(terminator - cursor) + 1;

  if (length > capacity) {
    return false;
  }

  memcpy(out, cursor, length);
  *offset += length;
  return true;
}

bool chat_packet_deserialize(const uint8_t *buffer, const size_t size,
                             ChatPacket *out) {
  if (buffer == nullptr || out == nullptr) {
    return false;
  }

  ChatPacketType type = 0;
  size_t payload_size = 0;

  if (!chat_packet_parse_header(buffer, size, &type, &payload_size)) {
    return false;
  }

  if (payload_size > size - CHAT_PACKET_HEADER_SIZE) {
    return false;
  }

  memset(out, 0, sizeof(*out));
  out->type = type;

  size_t offset = CHAT_PACKET_HEADER_SIZE;
  const size_t payload_end = CHAT_PACKET_HEADER_SIZE + payload_size;

  if (!read_string(buffer, payload_end, &offset, out->sender,
                   CHAT_NAME_CAPACITY)) {
    return false;
  }

  switch (type) {
  case CHAT_PACKET_JOIN:
  case CHAT_PACKET_LEAVE:
  case CHAT_PACKET_FILE_END: {
    break;
  }
  case CHAT_PACKET_TEXT: {
    return read_string(buffer, payload_end, &offset, out->text,
                       CHAT_TEXT_CAPACITY);
  }
  case CHAT_PACKET_FILE_META: {
    if (!read_string(buffer, payload_end, &offset, out->file_name,
                     CHAT_FILE_NAME_CAPACITY)) {
      return false;
    }

    if (offset + sizeof(out->file_size) > payload_end) {
      return false;
    }

    memcpy(&out->file_size, buffer + offset, sizeof(out->file_size));
    break;
  }
  case CHAT_PACKET_FILE_CHUNK: {
    out->chunk_data = buffer + offset;
    out->chunk_size = payload_end - offset;
    break;
  }
  default: {
    return false;
  }
  }

  return true;
}
