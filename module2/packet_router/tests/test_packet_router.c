#include "packet_router.h"
#include "unity.h"

void test_packet_router_is_same_subnet_run() {
  const uint32_t gateway = 0xC0A80101;  // 192.168.1.1
  const uint32_t mask = 0xFFFFFF00;     // 255.255.255.0
  const uint32_t ip_same = 0xC0A8010A;  // 192.168.1.10
  const uint32_t ip_other = 0xC0A8020A; // 192.168.2.10

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

  // Маска 0.0.0.0 — все IP попадают в «свою» подсеть.
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