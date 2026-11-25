#define _POSIX_C_SOURCE 202405L

#include "unity.h"

#include <netinet/in.h>
#include <string.h>

#include "raw_packet.h"

static constexpr uint32_t CLIENT_ADDRESS = 0x7f000001u;
static constexpr uint32_t SERVER_ADDRESS = 0x7f000002u;

static void test_build_parse_roundtrip() {
  const char *text = "ping";
  const UdpPacket packet = {
      .source_address = CLIENT_ADDRESS,
      .destination_address = SERVER_ADDRESS,
      .source_port = 41000,
      .destination_port = 5000,
      .payload = (const uint8_t *)text,
      .payload_length = strlen(text),
  };

  uint8_t buffer[128];
  const size_t length = raw_packet_build(buffer, sizeof(buffer), &packet);
  TEST_ASSERT_EQUAL_UINT64(20 + 8 + strlen(text), length);

  UdpPacket parsed = {0};
  TEST_ASSERT_TRUE(raw_packet_parse(buffer, length, &parsed));
  TEST_ASSERT_EQUAL_UINT32(CLIENT_ADDRESS, parsed.source_address);
  TEST_ASSERT_EQUAL_UINT32(SERVER_ADDRESS, parsed.destination_address);
  TEST_ASSERT_EQUAL_UINT16(41000, parsed.source_port);
  TEST_ASSERT_EQUAL_UINT16(5000, parsed.destination_port);
  TEST_ASSERT_EQUAL_UINT64(strlen(text), parsed.payload_length);
  TEST_ASSERT_EQUAL_MEMORY(text, parsed.payload, parsed.payload_length);
}

static void test_ip_header_checksum_is_valid() {
  const uint8_t payload[] = {0x01, 0x02, 0x03};
  const UdpPacket packet = {
      .source_address = CLIENT_ADDRESS,
      .destination_address = SERVER_ADDRESS,
      .source_port = 40000,
      .destination_port = 5000,
      .payload = payload,
      .payload_length = sizeof(payload),
  };

  uint8_t buffer[128];
  const size_t length = raw_packet_build(buffer, sizeof(buffer), &packet);
  TEST_ASSERT_TRUE(length > 0);
  TEST_ASSERT_EQUAL_UINT16(0, raw_packet_checksum(buffer, 20));
}

static void test_parse_rejects_malformed_packets() {
  uint8_t buffer[128];
  const uint8_t payload[] = {0x41};
  const UdpPacket packet = {
      .source_address = CLIENT_ADDRESS,
      .destination_address = SERVER_ADDRESS,
      .source_port = 40000,
      .destination_port = 5000,
      .payload = payload,
      .payload_length = sizeof(payload),
  };
  const size_t length = raw_packet_build(buffer, sizeof(buffer), &packet);
  TEST_ASSERT_TRUE(length > 0);

  UdpPacket parsed = {0};
  TEST_ASSERT_FALSE(raw_packet_parse(nullptr, length, &parsed));
  TEST_ASSERT_FALSE(raw_packet_parse(buffer, 10, &parsed));

  buffer[0] = 0x65;
  TEST_ASSERT_FALSE(raw_packet_parse(buffer, length, &parsed));
  buffer[0] = 0x45;

  buffer[9] = IPPROTO_TCP;
  TEST_ASSERT_FALSE(raw_packet_parse(buffer, length, &parsed));
  buffer[9] = IPPROTO_UDP;

  TEST_ASSERT_FALSE(raw_packet_parse(buffer, length - 1, &parsed));
}

static void test_build_rejects_invalid_arguments() {
  uint8_t buffer[64];
  const UdpPacket packet = {
      .source_address = CLIENT_ADDRESS,
      .destination_address = SERVER_ADDRESS,
      .source_port = 40000,
      .destination_port = 5000,
      .payload = (const uint8_t *)"x",
      .payload_length = 1,
  };

  TEST_ASSERT_EQUAL_UINT64(0,
                           raw_packet_build(nullptr, sizeof(buffer), &packet));
  TEST_ASSERT_EQUAL_UINT64(0,
                           raw_packet_build(buffer, sizeof(buffer), nullptr));
  TEST_ASSERT_EQUAL_UINT64(0, raw_packet_build(buffer, 8, &packet));

  UdpPacket without_payload = packet;
  without_payload.payload = nullptr;
  TEST_ASSERT_EQUAL_UINT64(
      0, raw_packet_build(buffer, sizeof(buffer), &without_payload));
}

void test_raw_packet_run() {
  RUN_TEST(test_build_parse_roundtrip);
  RUN_TEST(test_ip_header_checksum_is_valid);
  RUN_TEST(test_parse_rejects_malformed_packets);
  RUN_TEST(test_build_rejects_invalid_arguments);
}
