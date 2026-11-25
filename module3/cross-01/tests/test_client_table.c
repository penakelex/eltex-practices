#define _POSIX_C_SOURCE 202405L

#include "unity.h"

#include "client_table.h"

static constexpr uint32_t CLIENT_ONE = 0x7f000001u;
static constexpr uint32_t CLIENT_TWO = 0x7f000002u;

static void test_numbers_increment_per_client() {
  ClientTable table;
  client_table_initialize(&table);
  uint32_t number = 0;

  TEST_ASSERT_TRUE(
      client_table_next_number(&table, CLIENT_ONE, 40000, &number));
  TEST_ASSERT_EQUAL_UINT32(1, number);
  TEST_ASSERT_TRUE(
      client_table_next_number(&table, CLIENT_ONE, 40000, &number));
  TEST_ASSERT_EQUAL_UINT32(2, number);

  TEST_ASSERT_TRUE(
      client_table_next_number(&table, CLIENT_TWO, 40000, &number));
  TEST_ASSERT_EQUAL_UINT32(1, number);

  TEST_ASSERT_TRUE(
      client_table_next_number(&table, CLIENT_ONE, 40001, &number));
  TEST_ASSERT_EQUAL_UINT32(1, number);

  client_table_free(&table);
}

static void test_reset_restarts_counter() {
  ClientTable table;
  client_table_initialize(&table);
  uint32_t number = 0;

  client_table_next_number(&table, CLIENT_ONE, 40000, &number);
  client_table_next_number(&table, CLIENT_ONE, 40000, &number);
  client_table_reset(&table, CLIENT_ONE, 40000);

  TEST_ASSERT_TRUE(
      client_table_next_number(&table, CLIENT_ONE, 40000, &number));
  TEST_ASSERT_EQUAL_UINT32(1, number);

  client_table_free(&table);
}

static void test_reset_unknown_client_is_safe() {
  ClientTable table;
  client_table_initialize(&table);

  client_table_reset(&table, CLIENT_ONE, 40000);
  client_table_free(&table);
  client_table_free(&table);
}

static void test_null_arguments_rejected() {
  ClientTable table;
  client_table_initialize(&table);
  uint32_t number = 0;

  TEST_ASSERT_FALSE(
      client_table_next_number(nullptr, CLIENT_ONE, 40000, &number));
  TEST_ASSERT_FALSE(
      client_table_next_number(&table, CLIENT_ONE, 40000, nullptr));

  client_table_reset(nullptr, CLIENT_ONE, 40000);
  client_table_free(&table);
}

void test_client_table_run() {
  RUN_TEST(test_numbers_increment_per_client);
  RUN_TEST(test_reset_restarts_counter);
  RUN_TEST(test_reset_unknown_client_is_safe);
  RUN_TEST(test_null_arguments_rejected);
}
