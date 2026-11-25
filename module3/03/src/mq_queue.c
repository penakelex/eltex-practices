#define _POSIX_C_SOURCE 202405L

#include "mq_queue.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static bool open_owner_queues(MqChatQueues *queues) {
  queues->receive_queue =
      mq_open(queues->first_name, O_RDONLY | O_CREAT | O_EXCL | O_NONBLOCK,
              0600, nullptr);

  if (queues->receive_queue == (mqd_t)-1) {
    fprintf(stderr, "mq_open(%s): %s\n", queues->first_name, strerror(errno));
    return false;
  }

  queues->send_queue =
      mq_open(queues->second_name, O_WRONLY | O_CREAT | O_EXCL | O_NONBLOCK,
              0600, nullptr);

  if (queues->send_queue == (mqd_t)-1) {
    fprintf(stderr, "mq_open(%s): %s\n", queues->second_name, strerror(errno));
    mq_close(queues->receive_queue);
    mq_unlink(queues->first_name);
    return false;
  }

  queues->second_queue = (mqd_t)-1;
  queues->owner = true;
  return true;
}

static bool open_private_queue(MqChatQueues *queues, long pid) {
  mq_peer_queue_name(queues->private_name, sizeof(queues->private_name),
                     queues->base_name, pid);

  queues->receive_queue =
      mq_open(queues->private_name, O_RDONLY | O_NONBLOCK | O_CREAT | O_EXCL,
              0600, nullptr);

  if (queues->receive_queue == (mqd_t)-1 && errno == EEXIST) {
    mq_unlink(queues->private_name);
    queues->receive_queue =
        mq_open(queues->private_name, O_RDONLY | O_NONBLOCK | O_CREAT | O_EXCL,
                0600, nullptr);
  }

  if (queues->receive_queue == (mqd_t)-1) {
    fprintf(stderr, "mq_open(%s): %s\n", queues->private_name, strerror(errno));
    return false;
  }

  return true;
}

static bool open_peer_queues(MqChatQueues *queues, long pid) {
  queues->send_queue = mq_open(queues->first_name, O_WRONLY | O_NONBLOCK);

  if (queues->send_queue == (mqd_t)-1) {
    fprintf(stderr, "mq_open(%s): %s\n", queues->first_name, strerror(errno));
    return false;
  }

  queues->second_queue = mq_open(queues->second_name, O_RDONLY | O_NONBLOCK);

  if (queues->second_queue == (mqd_t)-1) {
    fprintf(stderr, "mq_open(%s): %s\n", queues->second_name, strerror(errno));
    mq_close(queues->send_queue);
    return false;
  }

  if (!open_private_queue(queues, pid)) {
    mq_close(queues->second_queue);
    mq_close(queues->send_queue);
    return false;
  }

  queues->owner = false;
  return true;
}

bool mq_chat_queues_open(MqChatQueues *queues, const char *base_name) {
  snprintf(queues->base_name, sizeof(queues->base_name), "%s", base_name);
  snprintf(queues->first_name, sizeof(queues->first_name), "%s_1", base_name);
  snprintf(queues->second_name, sizeof(queues->second_name), "%s_2", base_name);
  queues->private_name[0] = '\0';

  const mqd_t probe = mq_open(queues->first_name, O_WRONLY | O_NONBLOCK);

  if (probe != (mqd_t)-1) {
    mq_close(probe);
    return open_peer_queues(queues, (long)getpid());
  }

  if (errno != ENOENT) {
    fprintf(stderr, "mq_open(%s): %s\n", queues->first_name, strerror(errno));
    return false;
  }

  return open_owner_queues(queues);
}

void mq_chat_queues_close(MqChatQueues *queues) {
  mq_close(queues->receive_queue);
  mq_close(queues->send_queue);

  if (queues->owner) {
    mq_unlink(queues->first_name);
    mq_unlink(queues->second_name);
  } else {
    mq_close(queues->second_queue);
    mq_unlink(queues->private_name);
  }
}

void mq_peer_queue_name(char *name, size_t size, const char *base_name,
                        long pid) {
  snprintf(name, size, "%s_p%ld", base_name, pid);
}

mqd_t mq_peer_queue_open(const char *base_name, long pid) {
  char name[MQ_CHAT_NAME_MAX];
  mq_peer_queue_name(name, sizeof(name), base_name, pid);
  return mq_open(name, O_WRONLY | O_NONBLOCK);
}
