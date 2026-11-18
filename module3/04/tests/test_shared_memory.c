#define _POSIX_C_SOURCE 202405L

#include "semaphore.h"
#include "shared_memory.h"

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
  char key_path[128];
  make_key_path(key_path, sizeof(key_path), "shm");
  ensure_key_file(key_path);
  shared_memory_remove(key_path);

  SharedMemory created;
  bool created_flag = false;
  TEST_ASSERT_TRUE(shared_memory_create(&created, key_path, TEST_DATA_CAPACITY,
                                        &created_flag));
  TEST_ASSERT_TRUE(created_flag);
  TEST_ASSERT_EQUAL_size_t(TEST_DATA_CAPACITY,
                           shared_memory_data_capacity(&created));
  TEST_ASSERT_FALSE(shared_memory_is_producer_finished(&created));

  SharedMemory opened;
  TEST_ASSERT_TRUE(shared_memory_open(&opened, key_path, 0));
  TEST_ASSERT_EQUAL_size_t(TEST_DATA_CAPACITY,
                           shared_memory_data_capacity(&opened));

  shared_memory_set_producer_finished(&created);
  TEST_ASSERT_TRUE(shared_memory_is_producer_finished(&opened));

  memset(shared_memory_data(&created), 0xAB, TEST_DATA_CAPACITY);
  const unsigned char *peer =
      (const unsigned char *)shared_memory_data(&opened);
  TEST_ASSERT_EQUAL_HEX8(0xAB, peer[TEST_DATA_CAPACITY - 1]);

  shared_memory_close(&opened);
  shared_memory_close(&created);
  TEST_ASSERT_TRUE(shared_memory_remove(key_path));
  TEST_ASSERT_FALSE(shared_memory_remove(key_path));
  unlink(key_path);
}

void test_second_create_reports_existing_segment() {
  char key_path[128];
  make_key_path(key_path, sizeof(key_path), "shm2");
  ensure_key_file(key_path);
  shared_memory_remove(key_path);

  SharedMemory first;
  bool created_flag = false;
  TEST_ASSERT_TRUE(shared_memory_create(&first, key_path, TEST_DATA_CAPACITY,
                                        &created_flag));
  TEST_ASSERT_TRUE(created_flag);

  SharedMemory second;
  bool second_flag = true;
  TEST_ASSERT_TRUE(shared_memory_create(&second, key_path, TEST_DATA_CAPACITY,
                                        &second_flag));
  TEST_ASSERT_FALSE(second_flag);

  shared_memory_close(&second);
  shared_memory_close(&first);
  shared_memory_remove(key_path);
  unlink(key_path);
}

void test_semaphore_create_wait_post_remove() {
  char key_path[128];
  make_key_path(key_path, sizeof(key_path), "sem");
  ensure_key_file(key_path);
  semaphore_remove(key_path, 'S');

  Semaphore semaphore;
  bool created_flag = false;
  TEST_ASSERT_TRUE(semaphore_create(&semaphore, key_path, 'S', &created_flag));
  TEST_ASSERT_TRUE(created_flag);
  TEST_ASSERT_TRUE(semaphore_wait(&semaphore));
  TEST_ASSERT_TRUE(semaphore_post(&semaphore));

  Semaphore peer;
  TEST_ASSERT_TRUE(semaphore_open(&peer, key_path, 'S'));
  TEST_ASSERT_TRUE(semaphore_wait(&peer));
  TEST_ASSERT_TRUE(semaphore_post(&peer));

  TEST_ASSERT_TRUE(semaphore_remove(key_path, 'S'));
  TEST_ASSERT_FALSE(semaphore_remove(key_path, 'S'));
  unlink(key_path);
}

void test_shared_memory_run() {
  RUN_TEST(test_create_open_close_remove_shared_memory);
  RUN_TEST(test_second_create_reports_existing_segment);
  RUN_TEST(test_semaphore_create_wait_post_remove);
}
