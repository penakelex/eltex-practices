#define _POSIX_C_SOURCE 200809L

#include "packet_router.h"

#include "input_utils.h"

#include <arpa/inet.h>
#include <stdlib.h>

constexpr uint32_t RANDOM_MASK = 0xFFu;
constexpr uint32_t PREFIX_MIN = 0;
constexpr uint32_t PREFIX_MAX = 32;

static bool parse_ipv4(const char *str, uint32_t *out) {
  struct in_addr addr;

  if (inet_pton(AF_INET, str, &addr) != 1) {
    return false;
  }

  *out = ntohl(addr.s_addr);
  return true;
}

static bool parse_prefix_length(const char *str, uint32_t *out) {
  uint32_t value = 0;

  if (!parse_uint32(str, &value)) {
    return false;
  }

  if (value < PREFIX_MIN || value > PREFIX_MAX) {
    return false;
  }

  *out = value;
  return true;
}

static bool parse_mask(const char *str, uint32_t *out) {
  if (str == nullptr) {
    return false;
  }

  if (str[0] != '/') {
    return parse_ipv4(str, out);
  }

  uint32_t prefix = 0;

  if (!parse_prefix_length(str + 1, &prefix)) {
    return false;
  }

  if (prefix == 0) {
    *out = 0;
  } else {
    *out = 0xFFFFFFFFu << (32u - prefix);
  }

  return true;
}

static uint32_t generate_random_uint32() {
  const uint32_t first = (uint32_t)rand() & RANDOM_MASK;
  const uint32_t second = (uint32_t)rand() & RANDOM_MASK;
  const uint32_t third = (uint32_t)rand() & RANDOM_MASK;
  const uint32_t fourth = (uint32_t)rand() & RANDOM_MASK;

  return (first << 24) | (second << 16) | (third << 8) | fourth;
}

void packet_stats_initialize(PacketStats *stats) {
  if (stats == nullptr) {
    return;
  }

  stats->total_packets = 0;
  stats->local_packets = 0;
  stats->foreign_packets = 0;
}

void packet_stats_free(PacketStats *stats) {
  (void)stats;
}

bool packet_router_is_same_subnet(uint32_t ip, uint32_t gateway,
                                  uint32_t mask) {
  return (ip & mask) == (gateway & mask);
}

PacketRouterStatus packet_router_run(const char *gateway_str,
                                     const char *mask_str,
                                     uint32_t packet_count,
                                     PacketStats *stats) {
  if (gateway_str == nullptr || mask_str == nullptr || stats == nullptr) {
    return PACKET_ROUTER_INVALID_ARGUMENTS;
  }

  if (packet_count == 0) {
    return PACKET_ROUTER_INVALID_PACKET_COUNT;
  }

  uint32_t gateway = 0;
  uint32_t mask = 0;

  if (!parse_ipv4(gateway_str, &gateway)) {
    return PACKET_ROUTER_INVALID_GATEWAY;
  }

  if (!parse_mask(mask_str, &mask)) {
    return PACKET_ROUTER_INVALID_MASK;
  }

  for (uint32_t i = 0; i < packet_count; i++) {
    const uint32_t ip = generate_random_uint32();

    if (packet_router_is_same_subnet(ip, gateway, mask)) {
      stats->local_packets++;
    } else {
      stats->foreign_packets++;
    }

    stats->total_packets++;
  }

  return PACKET_ROUTER_OK;
}