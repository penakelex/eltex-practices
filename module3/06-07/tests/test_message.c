#define _POSIX_C_SOURCE 202405L

#include "message.h"

#include "unity.h"

void test_plain_text_is_text() {
  ChatInput input = {0};
  TEST_ASSERT_TRUE(chat_input_parse("hello world\n", &input));
  TEST_ASSERT_EQUAL_INT(CHAT_INPUT_TEXT, input.type);
  TEST_ASSERT_EQUAL_STRING("hello world", input.text);
}

void test_empty_line_is_empty() {
  ChatInput input = {0};
  TEST_ASSERT_TRUE(chat_input_parse("\n", &input));
  TEST_ASSERT_EQUAL_INT(CHAT_INPUT_EMPTY, input.type);

  TEST_ASSERT_TRUE(chat_input_parse("", &input));
  TEST_ASSERT_EQUAL_INT(CHAT_INPUT_EMPTY, input.type);
}

void test_file_command_is_file() {
  ChatInput input = {0};
  TEST_ASSERT_TRUE(chat_input_parse("/file /tmp/data.bin\n", &input));
  TEST_ASSERT_EQUAL_INT(CHAT_INPUT_FILE, input.type);
  TEST_ASSERT_EQUAL_STRING("/tmp/data.bin", input.file_path);
}

void test_file_command_without_path_is_empty() {
  ChatInput input = {0};
  TEST_ASSERT_TRUE(chat_input_parse("/file \n", &input));
  TEST_ASSERT_EQUAL_INT(CHAT_INPUT_EMPTY, input.type);
}

void test_unknown_command_is_rejected() {
  ChatInput input = {0};
  TEST_ASSERT_TRUE(chat_input_parse("/quit\n", &input));
  TEST_ASSERT_EQUAL_INT(CHAT_INPUT_UNKNOWN_COMMAND, input.type);
}

void test_null_arguments_are_rejected() {
  ChatInput input = {0};
  TEST_ASSERT_FALSE(chat_input_parse(nullptr, &input));
  TEST_ASSERT_FALSE(chat_input_parse("hello", nullptr));
}

void test_message_run() {
  RUN_TEST(test_plain_text_is_text);
  RUN_TEST(test_empty_line_is_empty);
  RUN_TEST(test_file_command_is_file);
  RUN_TEST(test_file_command_without_path_is_empty);
  RUN_TEST(test_unknown_command_is_rejected);
  RUN_TEST(test_null_arguments_are_rejected);
}
