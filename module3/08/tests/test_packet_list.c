#define _POSIX_C_SOURCE 202405L

#include "unity.h"

#include "packet_list.h"

static void test_list_add_grows_and_preserves_items() {
  PacketList list;
  packet_list_initialize(&list);

  for (uint16_t index = 0; index < 40; index++) {
    PacketInfo packet = {0};
    packet.source_port = index;
    TEST_ASSERT_TRUE(packet_list_add(&list, &packet));
  }

  TEST_ASSERT_EQUAL_UINT64(40, list.count);

  for (size_t index = 0; index < list.count; index++) {
    TEST_ASSERT_EQUAL_UINT16(index, list.items[index].source_port);
  }

  packet_list_free(&list);
  TEST_ASSERT_EQUAL_UINT64(0, list.count);
  TEST_ASSERT_NULL(list.items);

  packet_list_free(&list);
}

static void test_list_add_rejects_null() {
  PacketList list;
  packet_list_initialize(&list);
  PacketInfo packet = {0};

  TEST_ASSERT_FALSE(packet_list_add(nullptr, &packet));
  TEST_ASSERT_FALSE(packet_list_add(&list, nullptr));
  packet_list_free(&list);
}

void test_packet_list_run() {
  RUN_TEST(test_list_add_grows_and_preserves_items);
  RUN_TEST(test_list_add_rejects_null);
}
