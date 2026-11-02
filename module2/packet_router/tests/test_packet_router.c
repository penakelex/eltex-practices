#include "packet_router.h"
#include "unity.h"
#include <stdlib.h>

void test_packet_router_is_same_subnet_run() {
  const uint32_t gateway = 0xC0A80101;
  const uint32_t mask = 0xFFFFFF00;
  const uint32_t ip_same = 0xC0A8010A;
  const uint32_t ip_other = 0xC0A8020A;

  TEST_ASSERT_TRUE(packet_router_is_same_subnet(ip_same, gateway, mask));
  TEST_ASSERT_FALSE(packet_router_is_same_subnet(ip_other, gateway, mask));
}

void test_packet_router_run_invalid_run() {
  PacketStats stats;
  packet_stats_initialize(&stats);

  TEST_ASSERT_EQUAL_INT(
      PACKET_ROUTER_INVALID_ARGUMENTS,
      packet_router_run(nullptr, "255.255.255.0", 10, &stats));
  TEST_ASSERT_EQUAL_INT(PACKET_ROUTER_INVALID_ARGUMENTS,
                        packet_router_run("192.168.1.1", nullptr, 10, &stats));
  TEST_ASSERT_EQUAL_INT(
      PACKET_ROUTER_INVALID_ARGUMENTS,
      packet_router_run("192.168.1.1", "255.255.255.0", 10, nullptr));
  TEST_ASSERT_EQUAL_INT(
      PACKET_ROUTER_INVALID_PACKET_COUNT,
      packet_router_run("192.168.1.1", "255.255.255.0", 0, &stats));
  TEST_ASSERT_EQUAL_INT(
      PACKET_ROUTER_INVALID_GATEWAY,
      packet_router_run("invalid", "255.255.255.0", 10, &stats));
  TEST_ASSERT_EQUAL_INT(
      PACKET_ROUTER_INVALID_MASK,
      packet_router_run("192.168.1.1", "invalid", 10, &stats));
}

void test_packet_router_run_all_local_run() {
  PacketStats stats;
  packet_stats_initialize(&stats);

  const PacketRouterStatus status =
      packet_router_run("192.168.1.1", "0.0.0.0", 100, &stats);

  TEST_ASSERT_EQUAL_INT(PACKET_ROUTER_OK, status);
  TEST_ASSERT_EQUAL_UINT32(100, stats.total_packets);
  TEST_ASSERT_EQUAL_UINT32(100, stats.local_packets);
  TEST_ASSERT_EQUAL_UINT32(0, stats.foreign_packets);
}

void test_packet_stats_initialize_run() {
  PacketStats stats = {
      .total_packets = 1, .local_packets = 2, .foreign_packets = 3};

  packet_stats_initialize(&stats);

  TEST_ASSERT_EQUAL_UINT32(0, stats.total_packets);
  TEST_ASSERT_EQUAL_UINT32(0, stats.local_packets);
  TEST_ASSERT_EQUAL_UINT32(0, stats.foreign_packets);
}

void test_packet_router_cidr_mask_run() {
  PacketStats stats;

  packet_stats_initialize(&stats);
  TEST_ASSERT_EQUAL_INT(PACKET_ROUTER_OK,
                        packet_router_run("192.168.1.1", "/24", 100, &stats));
  TEST_ASSERT_EQUAL_UINT32(100, stats.total_packets);

  packet_stats_initialize(&stats);
  TEST_ASSERT_EQUAL_INT(PACKET_ROUTER_OK,
                        packet_router_run("192.168.1.1", "/0", 100, &stats));
  TEST_ASSERT_EQUAL_UINT32(100, stats.total_packets);
  TEST_ASSERT_EQUAL_UINT32(100, stats.local_packets);
  TEST_ASSERT_EQUAL_UINT32(0, stats.foreign_packets);

  packet_stats_initialize(&stats);
  TEST_ASSERT_EQUAL_INT(PACKET_ROUTER_OK,
                        packet_router_run("192.168.1.1", "/32", 100, &stats));
  TEST_ASSERT_EQUAL_UINT32(100, stats.total_packets);
}

void test_packet_router_cidr_mask_invalid_run() {
  PacketStats stats;
  packet_stats_initialize(&stats);

  TEST_ASSERT_EQUAL_INT(PACKET_ROUTER_INVALID_MASK,
                        packet_router_run("192.168.1.1", "/33", 100, &stats));
  TEST_ASSERT_EQUAL_INT(PACKET_ROUTER_INVALID_MASK,
                        packet_router_run("192.168.1.1", "/", 100, &stats));
  TEST_ASSERT_EQUAL_INT(PACKET_ROUTER_INVALID_MASK,
                        packet_router_run("192.168.1.1", "/abc", 100, &stats));
  TEST_ASSERT_EQUAL_INT(PACKET_ROUTER_INVALID_MASK,
                        packet_router_run("192.168.1.1", "/-1", 100, &stats));
  TEST_ASSERT_EQUAL_INT(
      PACKET_ROUTER_INVALID_MASK,
      packet_router_run("192.168.1.1", "/4294967296", 100, &stats));
}

void test_packet_router_cidr_equivalent_to_dotted_run() {
  PacketStats cidr;
  PacketStats dotted;

  srand(42);
  packet_stats_initialize(&cidr);
  packet_router_run("192.168.1.1", "/32", 1000, &cidr);

  srand(42);
  packet_stats_initialize(&dotted);
  packet_router_run("192.168.1.1", "255.255.255.255", 1000, &dotted);

  TEST_ASSERT_EQUAL_UINT32(cidr.total_packets, dotted.total_packets);
  TEST_ASSERT_EQUAL_UINT32(cidr.local_packets, dotted.local_packets);
  TEST_ASSERT_EQUAL_UINT32(cidr.foreign_packets, dotted.foreign_packets);
}