#define _POSIX_C_SOURCE 202405L

#include "subscriber.h"
#include "protocol.h"
#include "queue.h"
#include "signal_state.h"
#include "view.h"

#include <errno.h>
#include <unistd.h>

static void subscriber_send_control(const int msqid, const pid_t pid,
                                    const char *topic, const bool subscribe) {
  char text[QUEUE_MESSAGE_MAX_TEXT];
  bool formatted;

  if (subscribe) {
    formatted =
        protocol_message_format_subscribe(pid, topic, text, sizeof(text));
  } else {
    formatted =
        protocol_message_format_unsubscribe(pid, topic, text, sizeof(text));
  }

  if (formatted) {
    queue_send(msqid, QUEUE_MESSAGE_TYPE_CONTROL, text);
  }
}

bool subscriber_run(char *const *topics, const int topic_count) {
  if (topics == nullptr || topic_count <= 0) {
    return false;
  }

  signal_state_install_handlers();
  signal_state_reset();

  const auto msqid = queue_open();

  if (msqid < 0) {
    view_no_broker();
    return false;
  }

  const auto my_pid = getpid();

  for (int i = 0; i < topic_count; i++) {
    subscriber_send_control(msqid, my_pid, topics[i], true);
  }

  bool success = true;

  while (!signal_received) {
    QueueMessage message;
    const auto received = queue_receive(msqid, (long)my_pid, &message);

    if (received < 0) {
      if (errno == EINTR) {
        continue;
      }

      if (errno == EIDRM || errno == EINVAL) {
        view_queue_gone();
        success = false;
        break;
      }

      continue;
    }

    char topic[PROTOCOL_TOPIC_MAX];
    char payload[PROTOCOL_PAYLOAD_MAX];

    if (protocol_message_format_delivery_parse(
            message.text, topic, sizeof(topic), payload, sizeof(payload))) {
      view_subscriber_message(topic, payload);
    }
  }

  for (int i = 0; i < topic_count; i++) {
    subscriber_send_control(msqid, my_pid, topics[i], false);
  }

  return success;
}