#define _POSIX_C_SOURCE 200809L

#include "packet_router.h"

#include <arpa/inet.h>
#include <stdlib.h>

constexpr uint32_t RANDOM_MASK = 0xFFFF;

static bool parse_ipv4(const char *str, uint32_t *out) {
  struct in_addr addr;

  if (inet_pton(AF_INET, str, &addr) != 1) {
    return false;
  }

  *out = ntohl(addr.s_addr);
  return true;
}

static uint32_t generate_random_uint32() {
  const uint32_t high = (uint32_t)rand() & RANDOM_MASK;
  const uint32_t low = (uint32_t)rand() & RANDOM_MASK;

  return (high << 16) | low;
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

  if (!parse_ipv4(mask_str, &mask)) {
    return PACKET_ROUTER_INVALID_MASK;
  }

  packet_stats_initialize(stats);

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