#pragma once

#include <mqueue.h>

#include "chat.h"
#include "mq_queue.h"

bool chat_send_text(mqd_t queue, const char *text, unsigned int priority);

bool chat_owner_run(MqChatQueues *queues);

bool chat_peer_run(MqChatQueues *queues, long pid);
