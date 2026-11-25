#define _POSIX_C_SOURCE 202405L

#include "unity.h"

#include "packet_filter.h"

static PacketInfo make_packet(const uint16_t source_port,
                              const uint16_t destination_port) {
  PacketInfo packet = {0};
  packet.source_port = source_port;
  packet.destination_port = destination_port;
  return packet;
}

static void test_filter_parse_names() {
  FilterKind kind = FILTER_DNS;

  TEST_ASSERT_TRUE(packet_filter_parse("chat", &kind));
  TEST_ASSERT_EQUAL_INT(FILTER_CHAT, kind);
  TEST_ASSERT_TRUE(packet_filter_parse("dns", &kind));
  TEST_ASSERT_EQUAL_INT(FILTER_DNS, kind);
  TEST_ASSERT_TRUE(packet_filter_parse("dhcp", &kind));
  TEST_ASSERT_EQUAL_INT(FILTER_DHCP, kind);
  TEST_ASSERT_TRUE(packet_filter_parse("ntp", &kind));
  TEST_ASSERT_EQUAL_INT(FILTER_NTP, kind);
  TEST_ASSERT_TRUE(packet_filter_parse("mdns", &kind));
  TEST_ASSERT_EQUAL_INT(FILTER_MDNS, kind);
  TEST_ASSERT_TRUE(packet_filter_parse("quic", &kind));
  TEST_ASSERT_EQUAL_INT(FILTER_QUIC, kind);
  TEST_ASSERT_TRUE(packet_filter_parse("all", &kind));
  TEST_ASSERT_EQUAL_INT(FILTER_ALL, kind);
  TEST_ASSERT_FALSE(packet_filter_parse("http", &kind));
  TEST_ASSERT_FALSE(packet_filter_parse(nullptr, &kind));
  TEST_ASSERT_FALSE(packet_filter_parse("dns", nullptr));
}

static void test_filter_parse_combinations() {
  FilterKind kind = FILTER_NONE;

  TEST_ASSERT_TRUE(packet_filter_parse("dns,chat", &kind));
  TEST_ASSERT_EQUAL_INT(FILTER_DNS | FILTER_CHAT, kind);
  TEST_ASSERT_TRUE(packet_filter_parse("chat,ntp,quic", &kind));
  TEST_ASSERT_EQUAL_INT(FILTER_CHAT | FILTER_NTP | FILTER_QUIC, kind);
  TEST_ASSERT_FALSE(packet_filter_parse("", &kind));
  TEST_ASSERT_FALSE(packet_filter_parse("dns,", &kind));
  TEST_ASSERT_FALSE(packet_filter_parse("dns,http", &kind));
}

static void test_combined_filter_matches_any_component() {
  const FilterKind combined = FILTER_DNS | FILTER_CHAT;
  const PacketInfo dns = make_packet(53321, 53);
  const PacketInfo chat = make_packet(4000, 4000);
  const PacketInfo ntp = make_packet(123, 45678);

  TEST_ASSERT_TRUE(packet_filter_match(combined, &dns, 4000));
  TEST_ASSERT_TRUE(packet_filter_match(combined, &chat, 4000));
  TEST_ASSERT_FALSE(packet_filter_match(combined, &ntp, 4000));
}

static void test_chat_filter_matches_both_directions() {
  const PacketInfo outgoing = make_packet(50000, 4000);
  const PacketInfo incoming = make_packet(4000, 50000);
  const PacketInfo other = make_packet(50000, 50001);

  TEST_ASSERT_TRUE(packet_filter_match(FILTER_CHAT, &outgoing, 4000));
  TEST_ASSERT_TRUE(packet_filter_match(FILTER_CHAT, &incoming, 4000));
  TEST_ASSERT_FALSE(packet_filter_match(FILTER_CHAT, &other, 4000));
  TEST_ASSERT_FALSE(packet_filter_match(FILTER_CHAT, nullptr, 4000));
}

static void test_dns_filter_matches_port_53() {
  const PacketInfo query = make_packet(53321, 53);
  const PacketInfo response = make_packet(53, 53321);
  const PacketInfo other = make_packet(4000, 4000);

  TEST_ASSERT_TRUE(packet_filter_match(FILTER_DNS, &query, 4000));
  TEST_ASSERT_TRUE(packet_filter_match(FILTER_DNS, &response, 4000));
  TEST_ASSERT_FALSE(packet_filter_match(FILTER_DNS, &other, 4000));
}

static void test_service_filters_match_well_known_ports() {
  const PacketInfo dhcp = make_packet(68, 67);
  const PacketInfo ntp = make_packet(123, 45678);
  const PacketInfo mdns = make_packet(50000, 5353);
  const PacketInfo quic = make_packet(443, 50000);
  const PacketInfo other = make_packet(4000, 4000);

  TEST_ASSERT_TRUE(packet_filter_match(FILTER_DHCP, &dhcp, 4000));
  TEST_ASSERT_TRUE(packet_filter_match(FILTER_NTP, &ntp, 4000));
  TEST_ASSERT_TRUE(packet_filter_match(FILTER_MDNS, &mdns, 4000));
  TEST_ASSERT_TRUE(packet_filter_match(FILTER_QUIC, &quic, 4000));
  TEST_ASSERT_FALSE(packet_filter_match(FILTER_DHCP, &other, 4000));
  TEST_ASSERT_FALSE(packet_filter_match(FILTER_NTP, &other, 4000));
  TEST_ASSERT_FALSE(packet_filter_match(FILTER_MDNS, &other, 4000));
  TEST_ASSERT_FALSE(packet_filter_match(FILTER_QUIC, &other, 4000));
}

static void test_all_filter_matches_everything() {
  const PacketInfo packet = make_packet(4000, 4000);

  TEST_ASSERT_TRUE(packet_filter_match(FILTER_ALL, &packet, 4000));
  TEST_ASSERT_FALSE(packet_filter_match(FILTER_ALL, nullptr, 4000));
}

void test_packet_filter_run() {
  RUN_TEST(test_filter_parse_names);
  RUN_TEST(test_filter_parse_combinations);
  RUN_TEST(test_combined_filter_matches_any_component);
  RUN_TEST(test_chat_filter_matches_both_directions);
  RUN_TEST(test_dns_filter_matches_port_53);
  RUN_TEST(test_service_filters_match_well_known_ports);
  RUN_TEST(test_all_filter_matches_everything);
}
