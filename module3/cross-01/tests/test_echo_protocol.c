#define _POSIX_C_SOURCE 202405L

#include "unity.h"

#include <string.h>

#include "echo_protocol.h"

static void test_close_roundtrip() {
  uint8_t buffer[16];
  const size_t length = echo_protocol_build_close(buffer, sizeof(buffer));

  TEST_ASSERT_TRUE(length > 0);
  TEST_ASSERT_TRUE(echo_protocol_is_close(buffer, length));
}

static void test_regular_text_is_not_close() {
  const char *text = "ping";
  TEST_ASSERT_FALSE(
      echo_protocol_is_close((const uint8_t *)text, strlen(text)));
}

static void test_close_prefix_with_extra_byte_is_not_close() {
  uint8_t buffer[16];
  const size_t length = echo_protocol_build_close(buffer, sizeof(buffer));
  TEST_ASSERT_TRUE(length > 0);

  buffer[length] = 0x00;
  TEST_ASSERT_FALSE(echo_protocol_is_close(buffer, length + 1));
  TEST_ASSERT_FALSE(echo_protocol_is_close(buffer, length - 1));
}

static void test_close_rejects_null_and_small_capacity() {
  uint8_t buffer[16];

  TEST_ASSERT_FALSE(echo_protocol_is_close(nullptr, 0));
  TEST_ASSERT_EQUAL_UINT64(0, echo_protocol_build_close(nullptr, 16));
  TEST_ASSERT_EQUAL_UINT64(0, echo_protocol_build_close(buffer, 1));
}

void test_echo_protocol_run() {
  RUN_TEST(test_close_roundtrip);
  RUN_TEST(test_regular_text_is_not_close);
  RUN_TEST(test_close_prefix_with_extra_byte_is_not_close);
  RUN_TEST(test_close_rejects_null_and_small_capacity);
}
