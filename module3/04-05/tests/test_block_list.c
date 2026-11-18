#define _POSIX_C_SOURCE 202405L

#include "block_list.h"

#include <string.h>

#include "unity.h"

static char region[4096];

static void reset_region() {
  memset(region, 0, sizeof(region));
  block_list_initialize(region, sizeof(region));
}

void test_initialized_list_is_empty_and_all_processed() {
  reset_region();

  TEST_ASSERT_TRUE(block_list_is_empty(region));
  TEST_ASSERT_TRUE(block_list_all_processed(region, sizeof(region)));
}

void test_append_and_find_unprocessed() {
  reset_region();

  const int32_t first[] = {3, 1, 2};
  const int32_t second[] = {9, 5};

  uint32_t first_offset = 0;
  uint32_t second_offset = 0;
  TEST_ASSERT_TRUE(
      block_list_append(region, sizeof(region), first, 3, &first_offset));
  TEST_ASSERT_TRUE(
      block_list_append(region, sizeof(region), second, 2, &second_offset));
  TEST_ASSERT_FALSE(block_list_is_empty(region));

  uint32_t offset = 0;
  TEST_ASSERT_TRUE(
      block_list_find_unprocessed(region, sizeof(region), &offset));
  TEST_ASSERT_EQUAL_UINT32(first_offset, offset);

  const BlockHeader *header = block_list_header_at_const(region, first_offset);
  TEST_ASSERT_EQUAL_UINT32(3, header->count);
  TEST_ASSERT_EQUAL_UINT32(second_offset, header->next_offset);

  const int32_t *values = block_list_values_at_const(region, first_offset);
  TEST_ASSERT_EQUAL_INT32(3, values[0]);
  TEST_ASSERT_EQUAL_INT32(1, values[1]);
  TEST_ASSERT_EQUAL_INT32(2, values[2]);
}

void test_mark_processed_hides_block_from_scan() {
  reset_region();

  const int32_t first[] = {7};
  const int32_t second[] = {4, 8};

  uint32_t first_offset = 0;
  uint32_t second_offset = 0;
  TEST_ASSERT_TRUE(
      block_list_append(region, sizeof(region), first, 1, &first_offset));
  TEST_ASSERT_TRUE(
      block_list_append(region, sizeof(region), second, 2, &second_offset));

  block_list_mark_processed(region, first_offset);

  uint32_t offset = 0;
  TEST_ASSERT_TRUE(
      block_list_find_unprocessed(region, sizeof(region), &offset));
  TEST_ASSERT_EQUAL_UINT32(second_offset, offset);
  TEST_ASSERT_FALSE(block_list_all_processed(region, sizeof(region)));

  block_list_mark_processed(region, second_offset);
  TEST_ASSERT_TRUE(block_list_all_processed(region, sizeof(region)));
  TEST_ASSERT_FALSE(
      block_list_find_unprocessed(region, sizeof(region), &offset));
}

void test_append_fails_when_memory_exhausted() {
  char small[64];
  block_list_initialize(small, sizeof(small));

  const int32_t values[] = {1, 2, 3, 4, 5, 6};

  uint32_t offset = 0;
  TEST_ASSERT_TRUE(block_list_append(small, sizeof(small), values, 6, &offset));
  TEST_ASSERT_FALSE(
      block_list_append(small, sizeof(small), values, 6, &offset));
  TEST_ASSERT_FALSE(block_list_all_processed(small, sizeof(small)));

  block_list_mark_processed(small, offset);
  TEST_ASSERT_TRUE(block_list_all_processed(small, sizeof(small)));
}

void test_append_rejects_invalid_arguments() {
  reset_region();

  const int32_t values[] = {1};
  uint32_t offset = 0;

  TEST_ASSERT_FALSE(
      block_list_append(nullptr, sizeof(region), values, 1, &offset));
  TEST_ASSERT_FALSE(
      block_list_append(region, sizeof(region), nullptr, 1, &offset));
  TEST_ASSERT_FALSE(
      block_list_append(region, sizeof(region), values, 0, &offset));
  TEST_ASSERT_FALSE(
      block_list_append(region, sizeof(region), values, 1, nullptr));
}

void test_last_block_has_end_marker() {
  reset_region();

  const int32_t values[] = {42};
  uint32_t offset = 0;
  TEST_ASSERT_TRUE(
      block_list_append(region, sizeof(region), values, 1, &offset));

  const BlockHeader *header = block_list_header_at_const(region, offset);
  TEST_ASSERT_EQUAL_UINT32(BLOCK_LIST_END, header->next_offset);
}

void test_block_list_run() {
  RUN_TEST(test_initialized_list_is_empty_and_all_processed);
  RUN_TEST(test_append_and_find_unprocessed);
  RUN_TEST(test_mark_processed_hides_block_from_scan);
  RUN_TEST(test_append_fails_when_memory_exhausted);
  RUN_TEST(test_append_rejects_invalid_arguments);
  RUN_TEST(test_last_block_has_end_marker);
}
