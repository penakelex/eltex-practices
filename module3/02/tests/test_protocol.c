#define _POSIX_C_SOURCE 202405L

#include "protocol.h"
#include "unity.h"

#include <string.h>

void setUp() {
}

void tearDown() {
}

static void test_parse_send() {
  char text[256];
  TEST_ASSERT_TRUE(
      protocol_message_format_send(1234, "news", "hello", text, sizeof(text)));

  ProtocolMessage parsed;
  TEST_ASSERT_TRUE(protocol_message_parse(text, &parsed));
  TEST_ASSERT_EQUAL_INT(ProtocolMessageKindSend, parsed.kind);
  TEST_ASSERT_EQUAL_INT(1234, parsed.sender_pid);
  TEST_ASSERT_EQUAL_STRING("news", parsed.topic);
  TEST_ASSERT_EQUAL_STRING("hello", parsed.payload);
}

static void test_parse_subscribe() {
  char text[256];
  TEST_ASSERT_TRUE(
      protocol_message_format_subscribe(5678, "sports", text, sizeof(text)));

  ProtocolMessage parsed;
  TEST_ASSERT_TRUE(protocol_message_parse(text, &parsed));
  TEST_ASSERT_EQUAL_INT(ProtocolMessageKindSubscribe, parsed.kind);
  TEST_ASSERT_EQUAL_INT(5678, parsed.sender_pid);
  TEST_ASSERT_EQUAL_STRING("sports", parsed.topic);
  TEST_ASSERT_EQUAL_STRING("", parsed.payload);
}

static void test_parse_unsubscribe() {
  char text[256];
  TEST_ASSERT_TRUE(
      protocol_message_format_unsubscribe(9000, "weather", text, sizeof(text)));

  ProtocolMessage parsed;
  TEST_ASSERT_TRUE(protocol_message_parse(text, &parsed));
  TEST_ASSERT_EQUAL_INT(ProtocolMessageKindUnsubscribe, parsed.kind);
  TEST_ASSERT_EQUAL_INT(9000, parsed.sender_pid);
  TEST_ASSERT_EQUAL_STRING("weather", parsed.topic);
}

static void test_parse_malformed() {
  ProtocolMessage parsed;
  TEST_ASSERT_FALSE(protocol_message_parse("garbage", &parsed));
  TEST_ASSERT_FALSE(protocol_message_parse("send,,topic", &parsed));
  TEST_ASSERT_FALSE(protocol_message_parse("send,abc,topic", &parsed));
  TEST_ASSERT_FALSE(protocol_message_parse("subscribe,42", &parsed));
}

static void test_delivery_roundtrip() {
  char text[256];
  TEST_ASSERT_TRUE(protocol_message_format_delivery("news", "hello world", text,
                                                    sizeof(text)));

  char topic[64];
  char payload[128];
  TEST_ASSERT_TRUE(protocol_message_format_delivery_parse(
      text, topic, sizeof(topic), payload, sizeof(payload)));
  TEST_ASSERT_EQUAL_STRING("news", topic);
  TEST_ASSERT_EQUAL_STRING("hello world", payload);
}

static void test_delivery_empty_payload() {
  char text[64];
  TEST_ASSERT_TRUE(
      protocol_message_format_delivery("topic", "", text, sizeof(text)));

  char topic[32];
  char payload[32];
  TEST_ASSERT_TRUE(protocol_message_format_delivery_parse(
      text, topic, sizeof(topic), payload, sizeof(payload)));
  TEST_ASSERT_EQUAL_STRING("topic", topic);
  TEST_ASSERT_EQUAL_STRING("", payload);
}

void test_protocol_run() {
  RUN_TEST(test_parse_send);
  RUN_TEST(test_parse_subscribe);
  RUN_TEST(test_parse_unsubscribe);
  RUN_TEST(test_parse_malformed);
  RUN_TEST(test_delivery_roundtrip);
  RUN_TEST(test_delivery_empty_payload);
}

int main() {
  UNITY_BEGIN();
  test_protocol_run();
  return UNITY_END();
}