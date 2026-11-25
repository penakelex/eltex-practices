#define _POSIX_C_SOURCE 202405L

#include "packet_parser.h"

#include <arpa/inet.h>
#include <string.h>

static constexpr uint8_t IP_PROTOCOL_UDP = 17;
static constexpr uint16_t DNS_PORT = 53;
static constexpr size_t IPV4_HEADER_MINIMUM = 20;
static constexpr size_t UDP_HEADER_LENGTH = 8;
static constexpr size_t DNS_HEADER_LENGTH = 12;

static uint16_t read_uint16(const uint8_t *data) {
  uint16_t value = 0;
  memcpy(&value, data, sizeof(value));
  return ntohs(value);
}

static uint32_t read_uint32(const uint8_t *data) {
  uint32_t value = 0;
  memcpy(&value, data, sizeof(value));
  return value;
}

bool packet_parse_dns_name(const uint8_t *payload, const size_t length,
                           char *out, const size_t capacity) {
  if (payload == nullptr || out == nullptr || capacity == 0) {
    return false;
  }

  if (length < DNS_HEADER_LENGTH) {
    return false;
  }

  const uint16_t flags = read_uint16(payload + 2);
  const uint16_t questions = read_uint16(payload + 4);

  if ((flags & 0x8000) != 0 || questions == 0) {
    return false;
  }

  size_t position = DNS_HEADER_LENGTH;
  size_t written = 0;

  while (position < length) {
    const uint8_t label_length = payload[position];

    if (label_length == 0) {
      break;
    }

    if ((label_length & 0xC0) != 0 || position + 1 + label_length > length) {
      return false;
    }

    if (written > 0) {
      if (written + 1 >= capacity) {
        return false;
      }
      out[written++] = '.';
    }

    if (written + label_length >= capacity) {
      return false;
    }

    memcpy(out + written, payload + position + 1, label_length);
    written += label_length;
    position += 1 + label_length;
  }

  out[written] = '\0';
  return written > 0;
}

bool packet_parse(const uint8_t *data, const size_t length, PacketInfo *out) {
  if (data == nullptr || out == nullptr) {
    return false;
  }

  if (length < IPV4_HEADER_MINIMUM) {
    return false;
  }

  const uint8_t version = data[0] >> 4;
  const size_t header_length = (size_t)(data[0] & 0x0F) * 4;

  if (version != 4 || header_length < IPV4_HEADER_MINIMUM ||
      header_length > length) {
    return false;
  }

  if (data[9] != IP_PROTOCOL_UDP) {
    return false;
  }

  const uint16_t total_length = read_uint16(data + 2);

  if (total_length < header_length + UDP_HEADER_LENGTH ||
      total_length > length) {
    return false;
  }

  if (header_length + UDP_HEADER_LENGTH > total_length) {
    return false;
  }

  const size_t udp_offset = header_length;
  const size_t udp_length = total_length - udp_offset;
  const size_t payload_length = udp_length - UDP_HEADER_LENGTH;

  PacketInfo packet = {0};
  packet.source_address = read_uint32(data + 12);
  packet.destination_address = read_uint32(data + 16);
  packet.source_port = read_uint16(data + udp_offset);
  packet.destination_port = read_uint16(data + udp_offset + 2);
  packet.payload_length = payload_length;

  if (packet.source_port == DNS_PORT || packet.destination_port == DNS_PORT) {
    packet_parse_dns_name(data + udp_offset + UDP_HEADER_LENGTH, payload_length,
                          packet.dns_name, sizeof(packet.dns_name));
  }

  *out = packet;
  return true;
}
