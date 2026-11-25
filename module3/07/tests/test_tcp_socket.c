#define _POSIX_C_SOURCE 202405L

#include "tcp_socket.h"

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "protocol.h"
#include "unity.h"

static uint16_t listener_port(int fd) {
  struct sockaddr_in local = {0};
  socklen_t length = sizeof(local);
  getsockname(fd, (struct sockaddr *)&local, &length);
  return ntohs(local.sin_port);
}

void test_connect_send_and_receive() {
  const int listener = tcp_socket_create();
  TEST_ASSERT_TRUE(listener >= 0);
  TEST_ASSERT_TRUE(tcp_socket_set_reuse(listener));
  TEST_ASSERT_TRUE(tcp_socket_bind_listen(listener, 0, 4));

  const uint16_t port = listener_port(listener);
  TEST_ASSERT_TRUE(port > 0);

  const int client = tcp_socket_connect("127.0.0.1", port);
  TEST_ASSERT_TRUE(client >= 0);

  const int server_side = tcp_socket_accept(listener);
  TEST_ASSERT_TRUE(server_side >= 0);

  const char payload[] = "hello";
  TEST_ASSERT_TRUE(tcp_socket_send_all(client, payload, sizeof(payload)));

  char buffer[16] = {0};
  TEST_ASSERT_EQUAL_INT(
      sizeof(payload),
      tcp_socket_receive_all(server_side, buffer, sizeof(payload)));
  TEST_ASSERT_EQUAL_STRING("hello", buffer);

  close(client);
  close(server_side);
  close(listener);
}

void test_receive_packet_reads_full_frame() {
  int pair[2];
  TEST_ASSERT_EQUAL_INT(0, socketpair(AF_UNIX, SOCK_STREAM, 0, pair));

  ChatPacket packet = {.type = CHAT_PACKET_TEXT,
                       .file_size = 0,
                       .chunk_data = nullptr,
                       .chunk_size = 0};
  memcpy(packet.sender, "alice", 6);
  memcpy(packet.text, "framed", 7);

  uint8_t outgoing[CHAT_PACKET_BUFFER_SIZE];
  size_t outgoing_size = 0;
  TEST_ASSERT_TRUE(chat_packet_serialize(&packet, outgoing, sizeof(outgoing),
                                         &outgoing_size));
  TEST_ASSERT_TRUE(tcp_socket_send_all(pair[0], outgoing, outgoing_size));

  uint8_t incoming[CHAT_PACKET_BUFFER_SIZE];
  size_t incoming_size = 0;
  TEST_ASSERT_TRUE(tcp_socket_receive_packet(pair[1], incoming,
                                             sizeof(incoming), &incoming_size));
  TEST_ASSERT_EQUAL_UINT(outgoing_size, incoming_size);

  ChatPacket parsed = {0};
  TEST_ASSERT_TRUE(chat_packet_deserialize(incoming, incoming_size, &parsed));
  TEST_ASSERT_EQUAL_STRING("framed", parsed.text);

  close(pair[0]);
  close(pair[1]);
}

void test_receive_packet_fails_on_closed_peer() {
  int pair[2];
  TEST_ASSERT_EQUAL_INT(0, socketpair(AF_UNIX, SOCK_STREAM, 0, pair));
  close(pair[0]);

  uint8_t buffer[CHAT_PACKET_BUFFER_SIZE];
  size_t size = 0;
  TEST_ASSERT_FALSE(
      tcp_socket_receive_packet(pair[1], buffer, sizeof(buffer), &size));
  close(pair[1]);
}

void test_tcp_socket_run() {
  RUN_TEST(test_connect_send_and_receive);
  RUN_TEST(test_receive_packet_reads_full_frame);
  RUN_TEST(test_receive_packet_fails_on_closed_peer);
}
