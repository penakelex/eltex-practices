#pragma once

#include <stddef.h>
#include <stdint.h>

[[maybe_unused]] constexpr size_t PACKET_DNS_NAME_CAPACITY = 256;
[[maybe_unused]] constexpr size_t PACKET_MAC_LENGTH = 6;

typedef struct {
  double timestamp;
  uint8_t source_mac[PACKET_MAC_LENGTH];
  uint8_t destination_mac[PACKET_MAC_LENGTH];
  uint32_t source_address;
  uint32_t destination_address;
  uint16_t source_port;
  uint16_t destination_port;
  size_t payload_length;
  char dns_name[PACKET_DNS_NAME_CAPACITY];
} PacketInfo;

bool packet_parse(const uint8_t *data, const size_t length, PacketInfo *out);
bool packet_parse_dns_name(const uint8_t *payload, const size_t length,
                           char *out, const size_t capacity);
