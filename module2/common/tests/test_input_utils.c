#include "unity.h"
#include <input_utils.h>
#include <stdio.h>

static FILE *redirect_stdin_to_string(const char *contents) {
  FILE *temporary = tmpfile();

  if (temporary == NULL) {
    return NULL;
  }

  fputs(contents, temporary);
  rewind(temporary);
  return temporary;
}

static void test_read_line_strips_newline() {
  FILE *temporary = redirect_stdin_to_string("hello\n");
  TEST_ASSERT_NOT_NULL(temporary);

  if (temporary == NULL) {
    return;
  }

  FILE *saved_stdin = stdin;
  stdin = temporary;

  char buffer[32];
  read_line(buffer, sizeof buffer);

  stdin = saved_stdin;
  fclose(temporary);

  TEST_ASSERT_EQUAL_STRING("hello", buffer);
}

static void test_read_line_strips_carriage_return() {
  FILE *temporary = redirect_stdin_to_string("world\r\n");
  TEST_ASSERT_NOT_NULL(temporary);

  if (temporary == NULL) {
    return;
  }

  FILE *saved_stdin = stdin;
  stdin = temporary;

  char buffer[32];
  read_line(buffer, sizeof buffer);

  stdin = saved_stdin;
  fclose(temporary);

  TEST_ASSERT_EQUAL_STRING("world", buffer);
}

static void test_read_line_truncates_on_buffer_size() {
  FILE *temporary = redirect_stdin_to_string("abcdefghij\n");
  TEST_ASSERT_NOT_NULL(temporary);

  if (temporary == NULL) {
    return;
  }

  FILE *saved_stdin = stdin;
  stdin = temporary;

  char buffer[5];
  read_line(buffer, sizeof buffer);

  stdin = saved_stdin;
  fclose(temporary);

  TEST_ASSERT_EQUAL_STRING("abcd", buffer);
}

static void test_parse_uint32_valid() {
  uint32_t value = 0;

  TEST_ASSERT_TRUE(parse_uint32("0", &value));
  TEST_ASSERT_EQUAL_UINT32(0, value);

  TEST_ASSERT_TRUE(parse_uint32("123", &value));
  TEST_ASSERT_EQUAL_UINT32(123, value);

  TEST_ASSERT_TRUE(parse_uint32("4294967295", &value));
  TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, value);
}

static void test_parse_uint32_invalid() {
  uint32_t value = 0;

  TEST_ASSERT_FALSE(parse_uint32(NULL, &value));
  TEST_ASSERT_FALSE(parse_uint32("", &value));
  TEST_ASSERT_FALSE(parse_uint32("abc", &value));
  TEST_ASSERT_FALSE(parse_uint32("12a", &value));
  TEST_ASSERT_FALSE(parse_uint32("-5", &value));
  TEST_ASSERT_FALSE(parse_uint32("4294967296", &value));
  TEST_ASSERT_FALSE(parse_uint32("123", NULL));
}

void test_input_utils_run() {
  test_read_line_strips_newline();
  test_read_line_strips_carriage_return();
  test_read_line_truncates_on_buffer_size();
  test_parse_uint32_valid();
  test_parse_uint32_invalid();
}