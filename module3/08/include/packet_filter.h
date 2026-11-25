#pragma once

#include "packet_parser.h"

typedef enum : uint16_t {
  FILTER_NONE = 0,
  FILTER_CHAT = 1 << 0,
  FILTER_DNS = 1 << 1,
  FILTER_DHCP = 1 << 2,
  FILTER_NTP = 1 << 3,
  FILTER_MDNS = 1 << 4,
  FILTER_QUIC = 1 << 5,
  FILTER_ALL = (1 << 6) - 1,
} FilterKind;

bool packet_filter_parse(const char *names, FilterKind *out);
bool packet_filter_match(const FilterKind kind, const PacketInfo *packet,
                         const uint16_t chat_port);
