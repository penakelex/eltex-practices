#define _POSIX_C_SOURCE 202405L

#include "udp_socket.h"

#include <unistd.h>

#include "unity.h"

void test_send_and_receive_over_loopback() {
  const int receiver = udp_socket_create();
  const int sender = udp_socket_create();
  TEST_ASSERT_TRUE(receiver >= 0);
  TEST_ASSERT_TRUE(sender >= 0);

  TEST_ASSERT_TRUE(udp_socket_bind(receiver, "127.0.0.1", 0));

  uint16_t port = 0;
  TEST_ASSERT_TRUE(udp_socket_local_port(receiver, &port));
  TEST_ASSERT_TRUE(port > 0);

  struct sockaddr_in address = {0};
  TEST_ASSERT_TRUE(udp_socket_address(&address, "127.0.0.1", port));

  const char payload[] = "ping";
  TEST_ASSERT_TRUE(
      udp_socket_send_to(sender, &address, payload, sizeof(payload)));

  char buffer[64] = {0};
  const ssize_t received = udp_socket_receive(receiver, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_INT(sizeof(payload), received);
  TEST_ASSERT_EQUAL_STRING("ping", buffer);

  close(receiver);
  close(sender);
}

void test_broadcast_socket_supports_reuse() {
  const int first = udp_socket_create_broadcast();
  const int second = udp_socket_create_broadcast();
  TEST_ASSERT_TRUE(first >= 0);
  TEST_ASSERT_TRUE(second >= 0);

  TEST_ASSERT_TRUE(udp_socket_bind(first, nullptr, 0));
  TEST_ASSERT_TRUE(udp_socket_bind(second, nullptr, 0));

  close(first);
  close(second);
}

void test_invalid_address_is_rejected() {
  struct sockaddr_in address = {0};
  TEST_ASSERT_FALSE(udp_socket_address(&address, "not-an-address", 5000));
  TEST_ASSERT_FALSE(udp_socket_address(nullptr, "127.0.0.1", 5000));
}

void test_udp_socket_run() {
  RUN_TEST(test_send_and_receive_over_loopback);
  RUN_TEST(test_broadcast_socket_supports_reuse);
  RUN_TEST(test_invalid_address_is_rejected);
}
