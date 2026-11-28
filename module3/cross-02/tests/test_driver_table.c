#define _POSIX_C_SOURCE 202405L

#include "driver_table.h"

#include "unity.h"

static DriverEntry make_entry(const pid_t pid) {
  const DriverEntry entry = {.pid = pid,
                             .command_fd = 10,
                             .event_fd = 11,
                             .state = DRIVER_STATE_AVAILABLE};
  return entry;
}

void test_empty_table() {
  DriverTable table;
  driver_table_initialize(&table);

  TEST_ASSERT_EQUAL_UINT(0, driver_table_count(&table));
  TEST_ASSERT_NULL(driver_table_find(&table, 1));
  TEST_ASSERT_NULL(driver_table_at(&table, 0));

  driver_table_free(&table);
}

void test_add_and_find() {
  DriverTable table;
  driver_table_initialize(&table);

  TEST_ASSERT_TRUE(driver_table_add(&table, make_entry(100)));
  TEST_ASSERT_TRUE(driver_table_add(&table, make_entry(200)));
  TEST_ASSERT_EQUAL_UINT(2, driver_table_count(&table));

  DriverEntry *found = driver_table_find(&table, 200);
  TEST_ASSERT_NOT_NULL(found);
  TEST_ASSERT_EQUAL_INT(200, found->pid);

  driver_table_free(&table);
}

void test_growth_beyond_initial_capacity() {
  DriverTable table;
  driver_table_initialize(&table);

  for (pid_t pid = 1; pid <= 20; pid++) {
    TEST_ASSERT_TRUE(driver_table_add(&table, make_entry(pid)));
  }

  TEST_ASSERT_EQUAL_UINT(20, driver_table_count(&table));
  TEST_ASSERT_NOT_NULL(driver_table_find(&table, 20));

  driver_table_free(&table);
}

void test_remove_at() {
  DriverTable table;
  driver_table_initialize(&table);
  driver_table_add(&table, make_entry(100));
  driver_table_add(&table, make_entry(200));
  driver_table_add(&table, make_entry(300));

  driver_table_remove_at(&table, 1);
  TEST_ASSERT_EQUAL_UINT(2, driver_table_count(&table));
  TEST_ASSERT_NULL(driver_table_find(&table, 200));
  TEST_ASSERT_NOT_NULL(driver_table_find(&table, 300));

  driver_table_remove_at(&table, 5);
  TEST_ASSERT_EQUAL_UINT(2, driver_table_count(&table));

  driver_table_free(&table);
}

void test_free_is_idempotent() {
  DriverTable table;
  driver_table_initialize(&table);
  driver_table_add(&table, make_entry(100));

  driver_table_free(&table);
  driver_table_free(&table);
  TEST_ASSERT_EQUAL_UINT(0, driver_table_count(&table));
}

void test_driver_table_run() {
  RUN_TEST(test_empty_table);
  RUN_TEST(test_add_and_find);
  RUN_TEST(test_growth_beyond_initial_capacity);
  RUN_TEST(test_remove_at);
  RUN_TEST(test_free_is_idempotent);
}
