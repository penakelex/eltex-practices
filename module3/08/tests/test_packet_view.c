#define _POSIX_C_SOURCE 202405L

#include "unity.h"

#include <arpa/inet.h>
#include <string.h>

#include "packet_view.h"

static void test_view_format_full_line() {
  PacketInfo packet = {0};
  packet.timestamp = 12.345;
  const uint8_t source_mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
  const uint8_t destination_mac[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
  memcpy(packet.source_mac, source_mac, 6);
  memcpy(packet.destination_mac, destination_mac, 6);
  packet.source_address = inet_addr("192.168.1.2");
  packet.destination_address = inet_addr("192.168.1.255");
  packet.source_port = 4000;
  packet.destination_port = 4000;
  packet.payload_length = 42;

  char line[512] = {0};
  TEST_ASSERT_TRUE(packet_view_format(&packet, line, sizeof(line)));
  TEST_ASSERT_EQUAL_STRING(
      "+   12.345s MAC aa:bb:cc:dd:ee:ff -> 11:22:33:44:55:66 IP "
      "192.168.1.2:4000 -> 192.168.1.255:4000 len=42",
      line);
}

static void test_view_format_unknown_mac_and_dns() {
  PacketInfo packet = {0};
  packet.source_address = inet_addr("10.0.0.2");
  packet.destination_address = inet_addr("8.8.8.8");
  packet.source_port = 53321;
  packet.destination_port = 53;
  packet.payload_length = 30;
  snprintf(packet.dns_name, sizeof(packet.dns_name), "example.com");

  char line[512] = {0};
  TEST_ASSERT_TRUE(packet_view_format(&packet, line, sizeof(line)));
  TEST_ASSERT_NOT_NULL(strstr(line, "MAC ?? -> ??"));
  TEST_ASSERT_NOT_NULL(strstr(line, "dns=example.com"));
}

static void test_view_format_rejects_invalid() {
  PacketInfo packet = {0};
  char line[8] = {0};

  TEST_ASSERT_FALSE(packet_view_format(nullptr, line, sizeof(line)));
  TEST_ASSERT_FALSE(packet_view_format(&packet, nullptr, 0));
}

void test_packet_view_run() {
  RUN_TEST(test_view_format_full_line);
  RUN_TEST(test_view_format_unknown_mac_and_dns);
  RUN_TEST(test_view_format_rejects_invalid);
}
