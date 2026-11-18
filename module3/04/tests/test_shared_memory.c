#define _POSIX_C_SOURCE 202405L

#include "ipc_system_v.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "unity.h"

constexpr size_t TEST_DATA_CAPACITY = 256;

static void make_key_path(char *buffer, size_t size, const char *tag) {
  snprintf(buffer, size, "/tmp/m3t4_test_%s_%ld.key", tag, (long)getpid());
}

static void ensure_key_file(const char *key_path) {
  const int fd = open(key_path, O_WRONLY | O_CREAT, 0600);
  TEST_ASSERT_NOT_EQUAL(-1, fd);
  close(fd);
}

void test_create_open_close_remove_shared_memory() {
  const auto ops = ipc_system_v_ops();
  char key_path[128];
  make_key_path(key_path, sizeof(key_path), "shm");
  ensure_key_file(key_path);
  ipc_memory_remove(ops, key_path);

  Ipc ipc;
  ipc_initialize(&ipc, ops);
  bool created_flag = false;
  TEST_ASSERT_TRUE(
      ipc_memory_create(&ipc, key_path, TEST_DATA_CAPACITY, &created_flag));
  TEST_ASSERT_TRUE(created_flag);
  TEST_ASSERT_EQUAL_size_t(TEST_DATA_CAPACITY, ipc_memory_data_capacity(&ipc));
  TEST_ASSERT_FALSE(ipc_memory_is_producer_finished(&ipc));

  Ipc peer;
  ipc_initialize(&peer, ops);
  TEST_ASSERT_TRUE(ipc_memory_open(&peer, key_path));
  TEST_ASSERT_EQUAL_size_t(TEST_DATA_CAPACITY, ipc_memory_data_capacity(&peer));

  ipc_memory_set_producer_finished(&ipc);
  TEST_ASSERT_TRUE(ipc_memory_is_producer_finished(&peer));

  memset(ipc_memory_data(&ipc), 0xAB, TEST_DATA_CAPACITY);
  const unsigned char *peer_data =
      (const unsigned char *)ipc_memory_data(&peer);
  TEST_ASSERT_EQUAL_HEX8(0xAB, peer_data[TEST_DATA_CAPACITY - 1]);

  ipc_memory_close(&peer);
  ipc_memory_close(&ipc);
  TEST_ASSERT_TRUE(ipc_memory_remove(ops, key_path));
  TEST_ASSERT_FALSE(ipc_memory_remove(ops, key_path));
  unlink(key_path);
}

void test_second_create_reports_existing_segment() {
  const auto ops = ipc_system_v_ops();
  char key_path[128];
  make_key_path(key_path, sizeof(key_path), "shm2");
  ensure_key_file(key_path);
  ipc_memory_remove(ops, key_path);

  Ipc first;
  ipc_initialize(&first, ops);
  bool created_flag = false;
  TEST_ASSERT_TRUE(
      ipc_memory_create(&first, key_path, TEST_DATA_CAPACITY, &created_flag));
  TEST_ASSERT_TRUE(created_flag);

  Ipc second;
  ipc_initialize(&second, ops);
  bool second_flag = true;
  TEST_ASSERT_TRUE(
      ipc_memory_create(&second, key_path, TEST_DATA_CAPACITY, &second_flag));
  TEST_ASSERT_FALSE(second_flag);

  ipc_memory_close(&second);
  ipc_memory_close(&first);
  ipc_memory_remove(ops, key_path);
  unlink(key_path);
}

void test_semaphore_create_wait_post_remove() {
  const auto ops = ipc_system_v_ops();
  char key_path[128];
  make_key_path(key_path, sizeof(key_path), "sem");
  ensure_key_file(key_path);
  ipc_semaphore_remove(ops, key_path);

  Ipc ipc;
  ipc_initialize(&ipc, ops);
  bool created_flag = false;
  TEST_ASSERT_TRUE(ipc_semaphore_create(&ipc, key_path, &created_flag));
  TEST_ASSERT_TRUE(created_flag);
  TEST_ASSERT_TRUE(ipc_semaphore_wait(&ipc));
  TEST_ASSERT_TRUE(ipc_semaphore_post(&ipc));

  Ipc peer;
  ipc_initialize(&peer, ops);
  TEST_ASSERT_TRUE(ipc_semaphore_open(&peer, key_path));
  TEST_ASSERT_TRUE(ipc_semaphore_wait(&peer));
  TEST_ASSERT_TRUE(ipc_semaphore_post(&peer));

  TEST_ASSERT_TRUE(ipc_semaphore_remove(ops, key_path));
  TEST_ASSERT_FALSE(ipc_semaphore_remove(ops, key_path));
  unlink(key_path);
}

void test_shared_memory_run() {
  RUN_TEST(test_create_open_close_remove_shared_memory);
  RUN_TEST(test_second_create_reports_existing_segment);
  RUN_TEST(test_semaphore_create_wait_post_remove);
}
