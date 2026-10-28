#include "view.h"

#include <stdio.h>

void view_print_usage(const char *program_name) {
  printf("Usage: %s <gateway_ip> <subnet_mask> <packet_count>\n", program_name);
  printf("Example: %s 192.168.1.1 255.255.255.0 1000\n", program_name);
}

void view_print_error(PacketRouterStatus status) {
  switch (status) {
  case PACKET_ROUTER_INVALID_GATEWAY: {
    printf("Error: invalid gateway IP address.\n");
    break;
  }
  case PACKET_ROUTER_INVALID_MASK: {
    printf("Error: invalid subnet mask.\n");
    break;
  }
  case PACKET_ROUTER_INVALID_PACKET_COUNT: {
    printf("Error: packet count must be positive.\n");
    break;
  }
  case PACKET_ROUTER_INVALID_ARGUMENTS: {
    printf("Error: invalid arguments.\n");
    break;
  }
  default: {
    printf("Error: unknown error.\n");
    break;
  }
  }
}

void view_print_stats(const PacketStats *stats) {
  auto const total = (double)stats->total_packets;
  const double local_percent =
      total > 0.0 ? ((double)stats->local_packets / total) * 100.0 : 0.0;
  const double foreign_percent =
      total > 0.0 ? ((double)stats->foreign_packets / total) * 100.0 : 0.0;

  printf("Total packets: %u\n", stats->total_packets);
  printf("Packets to own subnet: %u (%.2f%%)\n", stats->local_packets,
         local_percent);
  printf("Packets to other networks: %u (%.2f%%)\n", stats->foreign_packets,
         foreign_percent);
}