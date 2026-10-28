#include "unity.h"
#include <stdlib.h>
#include <string_utils.h>

static void test_duplicate_string_normal() {
  const char *source = "hello";
  char *copy = duplicate_string(source);

  TEST_ASSERT_NOT_NULL(copy);
  TEST_ASSERT_EQUAL_STRING(source, copy);
  TEST_ASSERT_TRUE(copy != source);

  free(copy);
}

static void test_duplicate_string_empty() {
  char *copy = duplicate_string("");

  TEST_ASSERT_NOT_NULL(copy);
  TEST_ASSERT_EQUAL_STRING("", copy);

  free(copy);
}

static void test_duplicate_string_null() {
  TEST_ASSERT_NULL(duplicate_string(NULL));
}

void test_string_utils_run() {
  test_duplicate_string_normal();
  test_duplicate_string_empty();
  test_duplicate_string_null();
}