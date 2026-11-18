#pragma once

#include <mqueue.h>
#include <stddef.h>

[[maybe_unused]] constexpr size_t MQ_CHAT_NAME_MAX = 256;

typedef struct {
  mqd_t receive_queue;
  mqd_t send_queue;
  mqd_t second_queue;
  bool owner;
  char base_name[MQ_CHAT_NAME_MAX];
  char first_name[MQ_CHAT_NAME_MAX];
  char second_name[MQ_CHAT_NAME_MAX];
  char private_name[MQ_CHAT_NAME_MAX];
} MqChatQueues;

bool mq_chat_queues_open(MqChatQueues *queues, const char *base_name);

void mq_chat_queues_close(MqChatQueues *queues);

void mq_peer_queue_name(char *name, size_t size, const char *base_name,
                        long pid);

mqd_t mq_peer_queue_open(const char *base_name, long pid);
