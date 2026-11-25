#define _POSIX_C_SOURCE 202405L

#include "client_registry.h"

#include <sys/socket.h>
#include <unistd.h>

#include "unity.h"

void test_add_remove_and_contains() {
  ClientRegistry registry;
  client_registry_initialize(&registry);

  TEST_ASSERT_EQUAL_UINT(0, client_registry_size(&registry));
  TEST_ASSERT_TRUE(client_registry_add(&registry, 10));
  TEST_ASSERT_TRUE(client_registry_add(&registry, 20));
  TEST_ASSERT_TRUE(client_registry_add(&registry, 30));
  TEST_ASSERT_EQUAL_UINT(3, client_registry_size(&registry));
  TEST_ASSERT_TRUE(client_registry_contains(&registry, 20));

  TEST_ASSERT_TRUE(client_registry_remove(&registry, 20));
  TEST_ASSERT_FALSE(client_registry_contains(&registry, 20));
  TEST_ASSERT_EQUAL_UINT(2, client_registry_size(&registry));
  TEST_ASSERT_FALSE(client_registry_remove(&registry, 99));

  client_registry_free(&registry);
  TEST_ASSERT_EQUAL_UINT(0, client_registry_size(&registry));
}

void test_growth_beyond_initial_capacity() {
  ClientRegistry registry;
  client_registry_initialize(&registry);

  for (int fd = 0; fd < 32; fd += 1) {
    TEST_ASSERT_TRUE(client_registry_add(&registry, fd));
  }

  TEST_ASSERT_EQUAL_UINT(32, client_registry_size(&registry));
  TEST_ASSERT_TRUE(client_registry_contains(&registry, 31));

  client_registry_free(&registry);
}

void test_broadcast_skips_excluded_fd() {
  int first[2];
  int second[2];
  TEST_ASSERT_EQUAL_INT(0, socketpair(AF_UNIX, SOCK_STREAM, 0, first));
  TEST_ASSERT_EQUAL_INT(0, socketpair(AF_UNIX, SOCK_STREAM, 0, second));

  ClientRegistry registry;
  client_registry_initialize(&registry);
  client_registry_add(&registry, first[0]);
  client_registry_add(&registry, second[0]);

  const char payload[] = "data";
  client_registry_broadcast(&registry, first[0], payload, sizeof(payload));

  char buffer[16] = {0};
  TEST_ASSERT_EQUAL_INT(sizeof(payload),
                        recv(second[1], buffer, sizeof(buffer), 0));
  TEST_ASSERT_EQUAL_STRING("data", buffer);

  client_registry_free(&registry);
  close(first[0]);
  close(first[1]);
  close(second[0]);
  close(second[1]);
}

void test_null_arguments_are_safe() {
  ClientRegistry registry;
  client_registry_initialize(&registry);

  TEST_ASSERT_FALSE(client_registry_add(nullptr, 1));
  TEST_ASSERT_FALSE(client_registry_remove(nullptr, 1));
  TEST_ASSERT_FALSE(client_registry_contains(nullptr, 1));
  TEST_ASSERT_EQUAL_UINT(0, client_registry_size(nullptr));

  client_registry_broadcast(nullptr, -1, "x", 1);
  client_registry_broadcast(&registry, -1, nullptr, 0);
  client_registry_free(nullptr);

  client_registry_free(&registry);
}

void test_client_registry_run() {
  RUN_TEST(test_add_remove_and_contains);
  RUN_TEST(test_growth_beyond_initial_capacity);
  RUN_TEST(test_broadcast_skips_excluded_fd);
  RUN_TEST(test_null_arguments_are_safe);
}
