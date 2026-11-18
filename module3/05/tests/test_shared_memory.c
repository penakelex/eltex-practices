#define _POSIX_C_SOURCE 202405L

#include "ipc_posix.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "unity.h"

constexpr size_t TEST_DATA_CAPACITY = 256;

static void make_object_name(char *buffer, size_t size, const char *tag) {
  snprintf(buffer, size, "/m3t5_test_%s_%ld", tag, (long)getpid());
}

void test_create_open_close_remove_shared_memory() {
  const auto ops = ipc_posix_ops();
  char base_name[128];
  make_object_name(base_name, sizeof(base_name), "shm");
  ipc_memory_remove(ops, base_name);

  Ipc ipc;
  ipc_initialize(&ipc, ops);
  bool created_flag = false;
  TEST_ASSERT_TRUE(
      ipc_memory_create(&ipc, base_name, TEST_DATA_CAPACITY, &created_flag));
  TEST_ASSERT_TRUE(created_flag);
  TEST_ASSERT_EQUAL_size_t(TEST_DATA_CAPACITY, ipc_memory_data_capacity(&ipc));
  TEST_ASSERT_FALSE(ipc_memory_is_producer_finished(&ipc));

  Ipc peer;
  ipc_initialize(&peer, ops);
  TEST_ASSERT_TRUE(ipc_memory_open(&peer, base_name));
  TEST_ASSERT_EQUAL_size_t(TEST_DATA_CAPACITY, ipc_memory_data_capacity(&peer));

  ipc_memory_set_producer_finished(&ipc);
  TEST_ASSERT_TRUE(ipc_memory_is_producer_finished(&peer));

  memset(ipc_memory_data(&ipc), 0xAB, TEST_DATA_CAPACITY);
  const unsigned char *peer_data =
      (const unsigned char *)ipc_memory_data(&peer);
  TEST_ASSERT_EQUAL_HEX8(0xAB, peer_data[TEST_DATA_CAPACITY - 1]);

  ipc_memory_close(&peer);
  ipc_memory_close(&ipc);
  TEST_ASSERT_TRUE(ipc_memory_remove(ops, base_name));
  TEST_ASSERT_FALSE(ipc_memory_remove(ops, base_name));
}

void test_second_create_reports_existing_segment() {
  const auto ops = ipc_posix_ops();
  char base_name[128];
  make_object_name(base_name, sizeof(base_name), "shm2");
  ipc_memory_remove(ops, base_name);

  Ipc first;
  ipc_initialize(&first, ops);
  bool created_flag = false;
  TEST_ASSERT_TRUE(
      ipc_memory_create(&first, base_name, TEST_DATA_CAPACITY, &created_flag));
  TEST_ASSERT_TRUE(created_flag);

  Ipc second;
  ipc_initialize(&second, ops);
  bool second_flag = true;
  TEST_ASSERT_TRUE(
      ipc_memory_create(&second, base_name, TEST_DATA_CAPACITY, &second_flag));
  TEST_ASSERT_FALSE(second_flag);

  ipc_memory_close(&second);
  ipc_memory_close(&first);
  ipc_memory_remove(ops, base_name);
}

void test_semaphore_create_wait_post_remove() {
  const auto ops = ipc_posix_ops();
  char base_name[128];
  make_object_name(base_name, sizeof(base_name), "sem");
  ipc_semaphore_remove(ops, base_name);

  Ipc ipc;
  ipc_initialize(&ipc, ops);
  bool created_flag = false;
  TEST_ASSERT_TRUE(ipc_semaphore_create(&ipc, base_name, &created_flag));
  TEST_ASSERT_TRUE(created_flag);
  TEST_ASSERT_TRUE(ipc_semaphore_wait(&ipc));
  TEST_ASSERT_TRUE(ipc_semaphore_post(&ipc));

  Ipc peer;
  ipc_initialize(&peer, ops);
  TEST_ASSERT_TRUE(ipc_semaphore_open(&peer, base_name));
  TEST_ASSERT_TRUE(ipc_semaphore_wait(&peer));
  TEST_ASSERT_TRUE(ipc_semaphore_post(&peer));

  TEST_ASSERT_TRUE(ipc_semaphore_remove(ops, base_name));
  TEST_ASSERT_FALSE(ipc_semaphore_remove(ops, base_name));
}

void test_shared_memory_run() {
  RUN_TEST(test_create_open_close_remove_shared_memory);
  RUN_TEST(test_second_create_reports_existing_segment);
  RUN_TEST(test_semaphore_create_wait_post_remove);
}
