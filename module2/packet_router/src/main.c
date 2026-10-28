#include "packet_router.h"
#include "view.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
  if (argc != 4) {
    view_print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  const char *gateway_str = argv[1];
  const char *mask_str = argv[2];
  char *endptr = nullptr;
  const unsigned long packet_count_ul = strtoul(argv[3], &endptr, 10);

  if (endptr == argv[3] || *endptr != '\0' || packet_count_ul == 0 ||
      packet_count_ul > UINT32_MAX) {
    view_print_error(PACKET_ROUTER_INVALID_PACKET_COUNT);
    return EXIT_FAILURE;
  }

  const uint32_t packet_count = (uint32_t)packet_count_ul;
  PacketStats stats;

  packet_stats_initialize(&stats);
  srand((unsigned)time(nullptr));

  const PacketRouterStatus status =
      packet_router_run(gateway_str, mask_str, packet_count, &stats);

  if (status != PACKET_ROUTER_OK) {
    view_print_error(status);
    packet_stats_free(&stats);
    return EXIT_FAILURE;
  }

  view_print_stats(&stats);
  packet_stats_free(&stats);

  return EXIT_SUCCESS;
}