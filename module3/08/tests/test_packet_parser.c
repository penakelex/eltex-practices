#define _POSIX_C_SOURCE 202405L

#include "unity.h"

#include <arpa/inet.h>
#include <string.h>

#include "packet_parser.h"

static size_t build_datagram(uint8_t *buffer, const uint16_t source_port,
                             const uint16_t destination_port,
                             const uint8_t *payload,
                             const size_t payload_length,
                             const uint8_t protocol) {
  memset(buffer, 0, 64);
  buffer[0] = 0x45;
  const uint16_t total_length = htons((uint16_t)(20 + 8 + payload_length));
  memcpy(buffer + 2, &total_length, sizeof(total_length));
  buffer[9] = protocol;
  const uint32_t source = htonl(0xC0A80102);
  const uint32_t destination = htonl(0xC0A801FF);
  memcpy(buffer + 12, &source, sizeof(source));
  memcpy(buffer + 16, &destination, sizeof(destination));

  uint16_t port = htons(source_port);
  memcpy(buffer + 20, &port, sizeof(port));
  port = htons(destination_port);
  memcpy(buffer + 22, &port, sizeof(port));
  const uint16_t udp_length = htons((uint16_t)(8 + payload_length));
  memcpy(buffer + 24, &udp_length, sizeof(udp_length));

  if (payload_length > 0) {
    memcpy(buffer + 28, payload, payload_length);
  }

  return 20 + 8 + payload_length;
}

static size_t build_dns_payload(uint8_t *payload) {
  memset(payload, 0, 64);
  payload[2] = 0x01;
  payload[5] = 0x01;

  const char *name = "example.com";
  size_t position = 12;

  for (const char *part = name; *part != '\0';) {
    const char *dot = strchr(part, '.');
    const size_t length = dot != nullptr ? (size_t)(dot - part) : strlen(part);
    payload[position++] = (uint8_t)length;
    memcpy(payload + position, part, length);
    position += length;
    part += length;

    if (*part == '.') {
      part++;
    }
  }

  payload[position++] = 0;
  payload[position++] = 0;
  payload[position++] = 1;
  payload[position++] = 0;
  payload[position++] = 1;
  return position;
}

static void test_parse_chat_datagram() {
  uint8_t buffer[64] = {0};
  const uint8_t payload[] = {'h', 'e', 'l', 'l', 'o'};
  const size_t length =
      build_datagram(buffer, 4000, 4000, payload, sizeof(payload), 17);

  PacketInfo packet = {0};
  TEST_ASSERT_TRUE(packet_parse(buffer, length, &packet));
  TEST_ASSERT_EQUAL_UINT16(4000, packet.source_port);
  TEST_ASSERT_EQUAL_UINT16(4000, packet.destination_port);
  TEST_ASSERT_EQUAL_UINT32(htonl(0xC0A80102), packet.source_address);
  TEST_ASSERT_EQUAL_UINT32(htonl(0xC0A801FF), packet.destination_address);
  TEST_ASSERT_EQUAL_UINT64(sizeof(payload), packet.payload_length);
  TEST_ASSERT_EQUAL_STRING("", packet.dns_name);
}

static void test_parse_dns_datagram_extracts_name() {
  uint8_t payload[64] = {0};
  const size_t payload_length = build_dns_payload(payload);
  uint8_t buffer[128] = {0};
  const size_t length =
      build_datagram(buffer, 53321, 53, payload, payload_length, 17);

  PacketInfo packet = {0};
  TEST_ASSERT_TRUE(packet_parse(buffer, length, &packet));
  TEST_ASSERT_EQUAL_STRING("example.com", packet.dns_name);
}

static void test_parse_rejects_invalid_input() {
  uint8_t buffer[64] = {0};
  const size_t length = build_datagram(buffer, 1, 2, nullptr, 0, 17);
  PacketInfo packet = {0};

  TEST_ASSERT_FALSE(packet_parse(nullptr, length, &packet));
  TEST_ASSERT_FALSE(packet_parse(buffer, length, nullptr));
  TEST_ASSERT_FALSE(packet_parse(buffer, 10, &packet));

  buffer[0] = 0x65;
  TEST_ASSERT_FALSE(packet_parse(buffer, length, &packet));
  buffer[0] = 0x45;
  buffer[9] = 6;
  TEST_ASSERT_FALSE(packet_parse(buffer, length, &packet));
}

static void test_parse_dns_name_rejects_response() {
  uint8_t payload[64] = {0};
  size_t length = build_dns_payload(payload);
  payload[2] = 0x81;

  char name[PACKET_DNS_NAME_CAPACITY] = {0};
  TEST_ASSERT_FALSE(packet_parse_dns_name(payload, length, name, sizeof(name)));
}

static void test_parse_dns_name_rejects_truncated() {
  uint8_t payload[64] = {0};
  const size_t length = build_dns_payload(payload);

  char name[PACKET_DNS_NAME_CAPACITY] = {0};
  TEST_ASSERT_FALSE(packet_parse_dns_name(payload, 13, name, sizeof(name)));
  TEST_ASSERT_TRUE(packet_parse_dns_name(payload, length, name, sizeof(name)));
  TEST_ASSERT_EQUAL_STRING("example.com", name);
}

void test_packet_parser_run() {
  RUN_TEST(test_parse_chat_datagram);
  RUN_TEST(test_parse_dns_datagram_extracts_name);
  RUN_TEST(test_parse_rejects_invalid_input);
  RUN_TEST(test_parse_dns_name_rejects_response);
  RUN_TEST(test_parse_dns_name_rejects_truncated);
}
