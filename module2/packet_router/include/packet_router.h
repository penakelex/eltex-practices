#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  PACKET_ROUTER_OK = 0,
  PACKET_ROUTER_INVALID_GATEWAY,
  PACKET_ROUTER_INVALID_MASK,
  PACKET_ROUTER_INVALID_PACKET_COUNT,
  PACKET_ROUTER_INVALID_ARGUMENTS,
} PacketRouterStatus;

typedef struct {
  uint32_t total_packets;
  uint32_t local_packets;
  uint32_t foreign_packets;
} PacketStats;

void packet_stats_initialize(PacketStats *stats);
void packet_stats_free(PacketStats *stats);

PacketRouterStatus packet_router_run(const char *gateway_str,
                                     const char *mask_str,
                                     uint32_t packet_count, PacketStats *stats);

bool packet_router_is_same_subnet(uint32_t ip, uint32_t gateway, uint32_t mask);