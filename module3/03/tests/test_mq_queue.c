#define _POSIX_C_SOURCE 202405L

#include "mq_queue.h"
#include "protocol.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "unity.h"

void setUp() {
}

void tearDown() {
}

static void make_base_name(char *buffer, const size_t size) {
  snprintf(buffer, size, "/test_mq_chat_%ld", (long)getpid());
}

static void unlink_pair(const char *base_name) {
  char name[MQ_CHAT_NAME_MAX];

  snprintf(name, sizeof(name), "%s_1", base_name);
  mq_unlink(name);
  snprintf(name, sizeof(name), "%s_2", base_name);
  mq_unlink(name);
}

void test_owner_opens_and_unlinks_both_queues() {
  char base_name[MQ_CHAT_NAME_MAX];
  make_base_name(base_name, sizeof(base_name));
  unlink_pair(base_name);

  MqChatQueues queues;
  TEST_ASSERT_TRUE(mq_chat_queues_open(&queues, base_name));
  TEST_ASSERT_TRUE(queues.owner);

  char name[MQ_CHAT_NAME_MAX];
  snprintf(name, sizeof(name), "%s_1", base_name);
  mqd_t probe = mq_open(name, O_WRONLY | O_NONBLOCK);
  TEST_ASSERT_NOT_EQUAL((mqd_t)-1, probe);
  mq_close(probe);

  mq_chat_queues_close(&queues);

  snprintf(name, sizeof(name), "%s_1", base_name);
  probe = mq_open(name, O_WRONLY | O_NONBLOCK);
  TEST_ASSERT_EQUAL((mqd_t)-1, probe);
  TEST_ASSERT_EQUAL(ENOENT, errno);
}

void test_peer_opens_existing_pair_and_is_not_owner() {
  char base_name[MQ_CHAT_NAME_MAX];
  make_base_name(base_name, sizeof(base_name));
  unlink_pair(base_name);

  MqChatQueues owner;
  TEST_ASSERT_TRUE(mq_chat_queues_open(&owner, base_name));

  MqChatQueues peer;
  TEST_ASSERT_TRUE(mq_chat_queues_open(&peer, base_name));
  TEST_ASSERT_FALSE(peer.owner);
  TEST_ASSERT_NOT_EQUAL((mqd_t)-1, peer.second_queue);
  TEST_ASSERT_NOT_EQUAL((mqd_t)-1, peer.receive_queue);

  mq_chat_queues_close(&peer);

  char name[MQ_CHAT_NAME_MAX];
  snprintf(name, sizeof(name), "%s_1", base_name);
  mqd_t probe = mq_open(name, O_WRONLY | O_NONBLOCK);
  TEST_ASSERT_NOT_EQUAL((mqd_t)-1, probe);
  mq_close(probe);

  mq_chat_queues_close(&owner);
}

void test_second_peer_gets_a_private_queue() {
  char base_name[MQ_CHAT_NAME_MAX];
  make_base_name(base_name, sizeof(base_name));
  unlink_pair(base_name);

  char private_name[MQ_CHAT_NAME_MAX];
  mq_peer_queue_name(private_name, sizeof(private_name), base_name,
                     (long)getpid());

  MqChatQueues owner;
  MqChatQueues first;
  MqChatQueues second;
  TEST_ASSERT_TRUE(mq_chat_queues_open(&owner, base_name));
  TEST_ASSERT_TRUE(mq_chat_queues_open(&first, base_name));
  TEST_ASSERT_TRUE(mq_chat_queues_open(&second, base_name));
  TEST_ASSERT_EQUAL_STRING(private_name, second.private_name);

  mqd_t writer = mq_peer_queue_open(base_name, (long)getpid());
  TEST_ASSERT_NOT_EQUAL((mqd_t)-1, writer);
  TEST_ASSERT_EQUAL_INT(0, mq_send(writer, "hi", 3, 0));
  mq_close(writer);

  char buffer[8192];
  const ssize_t received =
      mq_receive(second.receive_queue, buffer, sizeof(buffer), nullptr);
  TEST_ASSERT_EQUAL((ssize_t)3, received);
  TEST_ASSERT_EQUAL_STRING("hi", buffer);

  mq_chat_queues_close(&second);
  mq_chat_queues_close(&first);

  mqd_t probe = mq_open(private_name, O_WRONLY | O_NONBLOCK);
  TEST_ASSERT_EQUAL((mqd_t)-1, probe);
  TEST_ASSERT_EQUAL(ENOENT, errno);

  mq_chat_queues_close(&owner);
}

void test_message_roundtrip_owner_to_peer() {
  char base_name[MQ_CHAT_NAME_MAX];
  make_base_name(base_name, sizeof(base_name));
  unlink_pair(base_name);

  MqChatQueues owner;
  MqChatQueues peer;
  TEST_ASSERT_TRUE(mq_chat_queues_open(&owner, base_name));
  TEST_ASSERT_TRUE(mq_chat_queues_open(&peer, base_name));

  const char *text = "hello from owner";
  TEST_ASSERT_EQUAL_INT(0,
                        mq_send(owner.send_queue, text, strlen(text) + 1, 0));

  char buffer[8192];
  unsigned int priority = 0;
  const ssize_t size =
      mq_receive(peer.second_queue, buffer, sizeof(buffer), &priority);
  TEST_ASSERT_EQUAL((ssize_t)(strlen(text) + 1), size);
  TEST_ASSERT_EQUAL_STRING(text, buffer);
  TEST_ASSERT_EQUAL_UINT(0, priority);

  mq_chat_queues_close(&peer);
  mq_chat_queues_close(&owner);
}

void test_quit_priority_is_preserved() {
  char base_name[MQ_CHAT_NAME_MAX];
  make_base_name(base_name, sizeof(base_name));
  unlink_pair(base_name);

  MqChatQueues owner;
  MqChatQueues peer;
  TEST_ASSERT_TRUE(mq_chat_queues_open(&owner, base_name));
  TEST_ASSERT_TRUE(mq_chat_queues_open(&peer, base_name));

  TEST_ASSERT_EQUAL_INT(0, mq_send(peer.send_queue, "quit", 5, 8));

  char buffer[8192];
  unsigned int priority = 0;
  const ssize_t size =
      mq_receive(owner.receive_queue, buffer, sizeof(buffer), &priority);
  TEST_ASSERT_EQUAL((ssize_t)5, size);
  TEST_ASSERT_EQUAL_STRING("quit", buffer);
  TEST_ASSERT_EQUAL_UINT(8, priority);

  mq_chat_queues_close(&peer);
  mq_chat_queues_close(&owner);
}

void test_hi_roundtrip() {
  char buffer[128];
  TEST_ASSERT_TRUE(protocol_format_hi(buffer, sizeof(buffer), 4242));

  long pid = 0;
  TEST_ASSERT_TRUE(protocol_parse_hi(buffer, &pid));
  TEST_ASSERT_EQUAL(4242, pid);
}

void test_msg_peer_roundtrip_keeps_commas_in_text() {
  char buffer[128];
  TEST_ASSERT_TRUE(
      protocol_format_msg_peer(buffer, sizeof(buffer), 77, "a,b, c"));

  long pid = 0;
  const char *text = nullptr;
  TEST_ASSERT_TRUE(protocol_parse_msg_peer(buffer, &pid, &text));
  TEST_ASSERT_EQUAL(77, pid);
  TEST_ASSERT_EQUAL_STRING("a,b, c", text);
}

void test_bye_peer_roundtrip() {
  char buffer[128];
  TEST_ASSERT_TRUE(protocol_format_bye_peer(buffer, sizeof(buffer), 123));

  long pid = 0;
  TEST_ASSERT_TRUE(protocol_parse_bye_peer(buffer, &pid));
  TEST_ASSERT_EQUAL(123, pid);
}

void test_you_assignment_matches_only_own_pid() {
  char buffer[128];
  TEST_ASSERT_TRUE(protocol_format_you(buffer, sizeof(buffer), 555, 3));

  long pid = 555;
  int peer_number = 0;
  char name[PROTOCOL_PEER_NAME_MAX];
  TEST_ASSERT_TRUE(
      protocol_parse_you(buffer, pid, &peer_number, name, sizeof(name)));
  TEST_ASSERT_EQUAL_INT(3, peer_number);
  TEST_ASSERT_EQUAL_STRING("peer3", name);

  TEST_ASSERT_FALSE(
      protocol_parse_you(buffer, 556, &peer_number, name, sizeof(name)));
}

void test_you_rejects_garbage_number() {
  char buffer[128];
  strcpy(buffer, "you,555,bogus");

  int peer_number = 0;
  char name[PROTOCOL_PEER_NAME_MAX];
  TEST_ASSERT_FALSE(
      protocol_parse_you(buffer, 555, &peer_number, name, sizeof(name)));
}

void test_msg_owner_roundtrip() {
  char buffer[128];
  TEST_ASSERT_TRUE(protocol_format_msg_owner(buffer, sizeof(buffer), "hi all"));

  const char *text = nullptr;
  TEST_ASSERT_TRUE(protocol_parse_msg_owner(buffer, &text));
  TEST_ASSERT_EQUAL_STRING("hi all", text);
}

void test_bye_owner_roundtrip() {
  char buffer[128];
  TEST_ASSERT_TRUE(protocol_format_bye_owner(buffer, sizeof(buffer)));
  TEST_ASSERT_EQUAL_STRING("bye", buffer);
}

void test_parsers_reject_wrong_prefix_and_junk() {
  char buffer[128];
  long pid = 0;
  const char *text = nullptr;
  int peer_number = 0;
  char name[PROTOCOL_PEER_NAME_MAX];

  strcpy(buffer, "msg,abc,hello");
  TEST_ASSERT_FALSE(protocol_parse_msg_peer(buffer, &pid, &text));

  strcpy(buffer, "hi,12x");
  TEST_ASSERT_FALSE(protocol_parse_hi(buffer, &pid));

  strcpy(buffer, "nonsense");
  TEST_ASSERT_FALSE(protocol_parse_hi(buffer, &pid));
  TEST_ASSERT_FALSE(protocol_parse_msg_owner(buffer, &text));
  TEST_ASSERT_FALSE(protocol_parse_bye_peer(buffer, &pid));
  TEST_ASSERT_FALSE(
      protocol_parse_you(buffer, 1, &peer_number, name, sizeof(name)));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_owner_opens_and_unlinks_both_queues);
  RUN_TEST(test_peer_opens_existing_pair_and_is_not_owner);
  RUN_TEST(test_second_peer_gets_a_private_queue);
  RUN_TEST(test_message_roundtrip_owner_to_peer);
  RUN_TEST(test_quit_priority_is_preserved);
  RUN_TEST(test_hi_roundtrip);
  RUN_TEST(test_msg_peer_roundtrip_keeps_commas_in_text);
  RUN_TEST(test_bye_peer_roundtrip);
  RUN_TEST(test_you_assignment_matches_only_own_pid);
  RUN_TEST(test_you_rejects_garbage_number);
  RUN_TEST(test_msg_owner_roundtrip);
  RUN_TEST(test_bye_owner_roundtrip);
  RUN_TEST(test_parsers_reject_wrong_prefix_and_junk);
  return UNITY_END();
}
