#define _POSIX_C_SOURCE 202405L

#include "protocol.h"

#include <string.h>

#include "unity.h"

static uint8_t buffer[8192];

void test_text_packet_roundtrip() {
  ChatPacket packet = {.type = CHAT_PACKET_TEXT,
                       .file_size = 0,
                       .chunk_data = nullptr,
                       .chunk_size = 0};
  memcpy(packet.sender, "alice", 6);
  memcpy(packet.text, "hello", 6);

  size_t size = 0;
  TEST_ASSERT_TRUE(
      chat_packet_serialize(&packet, buffer, sizeof(buffer), &size));

  ChatPacket parsed = {0};
  TEST_ASSERT_TRUE(chat_packet_deserialize(buffer, size, &parsed));
  TEST_ASSERT_EQUAL_INT(CHAT_PACKET_TEXT, parsed.type);
  TEST_ASSERT_EQUAL_STRING("alice", parsed.sender);
  TEST_ASSERT_EQUAL_STRING("hello", parsed.text);
}

void test_join_and_leave_roundtrip() {
  ChatPacket packet = {.type = CHAT_PACKET_JOIN,
                       .file_size = 0,
                       .chunk_data = nullptr,
                       .chunk_size = 0};
  memcpy(packet.sender, "bob", 4);

  size_t size = 0;
  TEST_ASSERT_TRUE(
      chat_packet_serialize(&packet, buffer, sizeof(buffer), &size));

  ChatPacket parsed = {0};
  TEST_ASSERT_TRUE(chat_packet_deserialize(buffer, size, &parsed));
  TEST_ASSERT_EQUAL_INT(CHAT_PACKET_JOIN, parsed.type);
  TEST_ASSERT_EQUAL_STRING("bob", parsed.sender);

  packet.type = CHAT_PACKET_LEAVE;
  TEST_ASSERT_TRUE(
      chat_packet_serialize(&packet, buffer, sizeof(buffer), &size));
  TEST_ASSERT_TRUE(chat_packet_deserialize(buffer, size, &parsed));
  TEST_ASSERT_EQUAL_INT(CHAT_PACKET_LEAVE, parsed.type);
}

void test_file_meta_roundtrip() {
  ChatPacket packet = {.type = CHAT_PACKET_FILE_META,
                       .file_size = 123456789,
                       .chunk_data = nullptr,
                       .chunk_size = 0};
  memcpy(packet.sender, "carol", 6);
  memcpy(packet.file_name, "report.pdf", 11);

  size_t size = 0;
  TEST_ASSERT_TRUE(
      chat_packet_serialize(&packet, buffer, sizeof(buffer), &size));

  ChatPacket parsed = {0};
  TEST_ASSERT_TRUE(chat_packet_deserialize(buffer, size, &parsed));
  TEST_ASSERT_EQUAL_INT(CHAT_PACKET_FILE_META, parsed.type);
  TEST_ASSERT_EQUAL_STRING("report.pdf", parsed.file_name);
  TEST_ASSERT_EQUAL_UINT64(123456789, parsed.file_size);
}

void test_file_chunk_roundtrip() {
  const uint8_t data[] = {0x01, 0x02, 0x00, 0x04};
  ChatPacket packet = {.type = CHAT_PACKET_FILE_CHUNK,
                       .file_size = 0,
                       .chunk_data = data,
                       .chunk_size = sizeof(data)};
  memcpy(packet.sender, "dave", 5);

  size_t size = 0;
  TEST_ASSERT_TRUE(
      chat_packet_serialize(&packet, buffer, sizeof(buffer), &size));

  ChatPacket parsed = {0};
  TEST_ASSERT_TRUE(chat_packet_deserialize(buffer, size, &parsed));
  TEST_ASSERT_EQUAL_INT(CHAT_PACKET_FILE_CHUNK, parsed.type);
  TEST_ASSERT_EQUAL_UINT(sizeof(data), parsed.chunk_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data, parsed.chunk_data, sizeof(data));
}

void test_serialize_rejects_small_buffer() {
  ChatPacket packet = {.type = CHAT_PACKET_TEXT,
                       .file_size = 0,
                       .chunk_data = nullptr,
                       .chunk_size = 0};
  memcpy(packet.sender, "alice", 6);
  memcpy(packet.text, "hello", 6);

  size_t size = 0;
  TEST_ASSERT_FALSE(chat_packet_serialize(&packet, buffer, 4, &size));
  TEST_ASSERT_FALSE(
      chat_packet_serialize(nullptr, buffer, sizeof(buffer), &size));
}

void test_deserialize_rejects_invalid_input() {
  ChatPacket parsed = {0};

  TEST_ASSERT_FALSE(chat_packet_deserialize(nullptr, 10, &parsed));
  TEST_ASSERT_FALSE(chat_packet_deserialize(buffer, 2, &parsed));

  ChatPacket packet = {.type = CHAT_PACKET_TEXT,
                       .file_size = 0,
                       .chunk_data = nullptr,
                       .chunk_size = 0};
  memcpy(packet.sender, "alice", 6);

  size_t size = 0;
  TEST_ASSERT_TRUE(
      chat_packet_serialize(&packet, buffer, sizeof(buffer), &size));
  TEST_ASSERT_FALSE(chat_packet_deserialize(buffer, size - 1, &parsed));
}

void test_parse_header_extracts_type_and_size() {
  ChatPacket packet = {.type = CHAT_PACKET_JOIN,
                       .file_size = 0,
                       .chunk_data = nullptr,
                       .chunk_size = 0};
  memcpy(packet.sender, "eve", 4);

  size_t size = 0;
  TEST_ASSERT_TRUE(
      chat_packet_serialize(&packet, buffer, sizeof(buffer), &size));

  ChatPacketType type = 0;
  size_t payload_size = 0;
  TEST_ASSERT_TRUE(
      chat_packet_parse_header(buffer, size, &type, &payload_size));
  TEST_ASSERT_EQUAL_INT(CHAT_PACKET_JOIN, type);
  TEST_ASSERT_EQUAL_UINT(size - CHAT_PACKET_HEADER_SIZE, payload_size);
}

void test_protocol_run() {
  RUN_TEST(test_text_packet_roundtrip);
  RUN_TEST(test_join_and_leave_roundtrip);
  RUN_TEST(test_file_meta_roundtrip);
  RUN_TEST(test_file_chunk_roundtrip);
  RUN_TEST(test_serialize_rejects_small_buffer);
  RUN_TEST(test_deserialize_rejects_invalid_input);
  RUN_TEST(test_parse_header_extracts_type_and_size);
}
