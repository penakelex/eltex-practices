#define _POSIX_C_SOURCE 202405L

#include "chat_internal.h"
#include "signal_state.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

bool chat_send_text(mqd_t queue, const char *text, unsigned int priority) {
  if (mq_send(queue, text, strlen(text) + 1, priority) == -1) {
    fprintf(stderr, "mq_send: %s\n", strerror(errno));
    return false;
  }

  return true;
}

bool chat_run(const char *queue_name) {
  MqChatQueues queues;

  if (!mq_chat_queues_open(&queues, queue_name)) {
    return false;
  }

  signal_state_install_handlers();

  const bool result = queues.owner ? chat_owner_run(&queues)
                                   : chat_peer_run(&queues, (long)getpid());

  mq_chat_queues_close(&queues);
  return result;
}
