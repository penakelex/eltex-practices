#include "unity.h"
#include <fields.h>

static void test_single_field() {
  TEST_ASSERT_EQUAL_UINT32(FIELD_LAST_NAME, fields_parse_selection("1"));
  TEST_ASSERT_EQUAL_UINT32(FIELD_FIRST_NAME, fields_parse_selection("2"));
  TEST_ASSERT_EQUAL_UINT32(FIELD_MIDDLE_NAME, fields_parse_selection("3"));
  TEST_ASSERT_EQUAL_UINT32(FIELD_WORKPLACE, fields_parse_selection("4"));
  TEST_ASSERT_EQUAL_UINT32(FIELD_POSITION, fields_parse_selection("5"));
  TEST_ASSERT_EQUAL_UINT32(FIELD_PHONES, fields_parse_selection("6"));
  TEST_ASSERT_EQUAL_UINT32(FIELD_EMAILS, fields_parse_selection("7"));
  TEST_ASSERT_EQUAL_UINT32(FIELD_SOCIAL_NETWORKS, fields_parse_selection("8"));
  TEST_ASSERT_EQUAL_UINT32(FIELD_MESSENGERS, fields_parse_selection("9"));
}

static void test_multiple_fields() {
  const uint32_t expected = FIELD_LAST_NAME | FIELD_FIRST_NAME | FIELD_POSITION;

  TEST_ASSERT_EQUAL_UINT32(expected, fields_parse_selection("1,2,5"));
  TEST_ASSERT_EQUAL_UINT32(expected, fields_parse_selection("1, 2, 5"));
  TEST_ASSERT_EQUAL_UINT32(expected, fields_parse_selection("1 2 5"));
}

static void test_all_fields() {
  TEST_ASSERT_EQUAL_UINT32(FIELD_ALL, fields_parse_selection("0"));
}

static void test_empty_input() {
  TEST_ASSERT_EQUAL_UINT32(0, fields_parse_selection(""));
  TEST_ASSERT_EQUAL_UINT32(0, fields_parse_selection("   "));
}

static void test_invalid_input() {
  TEST_ASSERT_EQUAL_UINT32(0, fields_parse_selection("abc"));
  TEST_ASSERT_EQUAL_UINT32(0, fields_parse_selection("10"));
  TEST_ASSERT_EQUAL_UINT32(0, fields_parse_selection("1,x"));
}

void test_fields_run() {
  test_single_field();
  test_multiple_fields();
  test_all_fields();
  test_empty_input();
  test_invalid_input();
}