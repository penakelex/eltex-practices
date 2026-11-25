#define _POSIX_C_SOURCE 202405L

#include "unity.h"

#include <arpa/inet.h>
#include <string.h>

#include "mac_resolver.h"

static const char ARP_TABLE[] =
    "IP address       HW type     Flags       HW address            Mask     "
    "Device\n"
    "192.168.1.1      0x1         0x2         aa:bb:cc:dd:ee:ff     *        "
    "eth0\n"
    "10.0.0.2         0x1         0x2         11:22:33:44:55:66     *        "
    "eth0\n";

static void test_mac_parse_valid() {
  uint8_t mac[6] = {0};
  TEST_ASSERT_TRUE(mac_parse("aa:bb:cc:dd:ee:ff", mac));

  const uint8_t expected[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
  TEST_ASSERT_EQUAL_MEMORY(expected, mac, 6);
}

static void test_mac_parse_invalid() {
  uint8_t mac[6] = {0};

  TEST_ASSERT_FALSE(mac_parse(nullptr, mac));
  TEST_ASSERT_FALSE(mac_parse("aa:bb:cc", mac));
  TEST_ASSERT_FALSE(mac_parse("zz:bb:cc:dd:ee:ff", mac));
  TEST_ASSERT_FALSE(mac_parse("aa:bb:cc:dd:ee:ff:00", mac));
  TEST_ASSERT_FALSE(mac_parse("aa:bb:cc:dd:ee:ff", nullptr));
}

static void test_arp_lookup_finds_entry() {
  uint8_t mac[6] = {0};
  const uint32_t address = inet_addr("192.168.1.1");

  TEST_ASSERT_TRUE(mac_arp_lookup(ARP_TABLE, strlen(ARP_TABLE), address, mac));

  const uint8_t expected[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
  TEST_ASSERT_EQUAL_MEMORY(expected, mac, 6);
}

static void test_arp_lookup_misses_unknown() {
  uint8_t mac[6] = {0};
  const uint32_t address = inet_addr("172.16.0.1");

  TEST_ASSERT_FALSE(mac_arp_lookup(ARP_TABLE, strlen(ARP_TABLE), address, mac));
  TEST_ASSERT_FALSE(mac_arp_lookup(nullptr, 0, address, mac));
}

static void test_resolve_broadcast_is_ff() {
  uint8_t mac[6] = {0};
  TEST_ASSERT_TRUE(mac_resolve(htonl(INADDR_BROADCAST), mac));

  const uint8_t expected[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  TEST_ASSERT_EQUAL_MEMORY(expected, mac, 6);
}

void test_mac_resolver_run() {
  RUN_TEST(test_mac_parse_valid);
  RUN_TEST(test_mac_parse_invalid);
  RUN_TEST(test_arp_lookup_finds_entry);
  RUN_TEST(test_arp_lookup_misses_unknown);
  RUN_TEST(test_resolve_broadcast_is_ff);
}
