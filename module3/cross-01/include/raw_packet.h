#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint32_t source_address;
  uint32_t destination_address;
  uint16_t source_port;
  uint16_t destination_port;
  const uint8_t *payload;
  size_t payload_length;
} UdpPacket;

uint16_t raw_packet_checksum(const void *data, size_t length);
size_t raw_packet_build(uint8_t *buffer, size_t capacity,
                        const UdpPacket *packet);
bool raw_packet_parse(const uint8_t *buffer, size_t length, UdpPacket *packet);
