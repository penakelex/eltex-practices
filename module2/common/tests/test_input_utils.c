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

void test_input_utils_run() {
  test_read_line_strips_newline();
  test_read_line_strips_carriage_return();
  test_read_line_truncates_on_buffer_size();
}