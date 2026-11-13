#define _POSIX_C_SOURCE 202405L

#include "file_copy.h"
#include "unity.h"

#include <string.h>
#include <unistd.h>

void setUp() {
}

void tearDown() {
}

static void test_header_roundtrip() {
  int fds[2];
  TEST_ASSERT_EQUAL_INT(0, pipe(fds));

  TEST_ASSERT_TRUE(file_copy_write_header(fds[1], "test.txt", 1234));
  close(fds[1]);

  char name[FILE_COPY_MAX_NAME_LENGTH + 1];
  uint32_t name_length = 0;
  uint64_t size = 0;

  TEST_ASSERT_TRUE(
      file_copy_read_header(fds[0], name, sizeof(name), &name_length, &size));
  close(fds[0]);

  TEST_ASSERT_EQUAL_UINT32(strlen("test.txt"), name_length);
  TEST_ASSERT_EQUAL_STRING("test.txt", name);
  TEST_ASSERT_EQUAL_UINT64(1234, size);
}

static void test_termination_header() {
  int fds[2];
  TEST_ASSERT_EQUAL_INT(0, pipe(fds));

  TEST_ASSERT_TRUE(file_copy_write_header(fds[1], nullptr, 0));
  close(fds[1]);

  char name[FILE_COPY_MAX_NAME_LENGTH + 1];
  uint32_t name_length = 1;
  uint64_t size = 1;

  TEST_ASSERT_TRUE(
      file_copy_read_header(fds[0], name, sizeof(name), &name_length, &size));
  close(fds[0]);

  TEST_ASSERT_EQUAL_UINT32(0, name_length);
  TEST_ASSERT_EQUAL_STRING("", name);
  TEST_ASSERT_EQUAL_UINT64(0, size);
}

static void test_empty_name_is_termination() {
  int fds[2];
  TEST_ASSERT_EQUAL_INT(0, pipe(fds));

  TEST_ASSERT_TRUE(file_copy_write_header(fds[1], "", 42));
  close(fds[1]);

  char name[FILE_COPY_MAX_NAME_LENGTH + 1];
  uint32_t name_length = 1;
  uint64_t size = 1;

  TEST_ASSERT_TRUE(
      file_copy_read_header(fds[0], name, sizeof(name), &name_length, &size));
  close(fds[0]);

  TEST_ASSERT_EQUAL_UINT32(0, name_length);
  TEST_ASSERT_EQUAL_UINT64(0, size);
}

static void test_header_name_too_long() {
  int fds[2];
  TEST_ASSERT_EQUAL_INT(0, pipe(fds));

  TEST_ASSERT_TRUE(file_copy_write_header(fds[1], "somefile", 10));
  close(fds[1]);

  char name[4];
  uint32_t name_length = 0;
  uint64_t size = 0;

  TEST_ASSERT_FALSE(
      file_copy_read_header(fds[0], name, sizeof(name), &name_length, &size));
  close(fds[0]);
}

void test_file_copy_run() {
  RUN_TEST(test_header_roundtrip);
  RUN_TEST(test_termination_header);
  RUN_TEST(test_empty_name_is_termination);
  RUN_TEST(test_header_name_too_long);
}

int main() {
  UNITY_BEGIN();
  test_file_copy_run();
  return UNITY_END();
}