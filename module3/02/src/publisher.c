#define _POSIX_C_SOURCE 202405L

#include "publisher.h"
#include "protocol.h"
#include "queue.h"
#include "signal_state.h"
#include "view.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

bool publisher_run(const char *topic) {
  if (topic == nullptr) {
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

  char *line = nullptr;
  size_t line_capacity = 0;
  bool success = true;

  while (!signal_received) {
    const auto line_length = getline(&line, &line_capacity, stdin);

    if (line_length < 0) {
      break;
    }

    if (line_length > 0 && line[line_length - 1] == '\n') {
      line[line_length - 1] = '\0';
    }

    if (line[0] == '\0') {
      continue;
    }

    char message_text[QUEUE_MESSAGE_MAX_TEXT];

    if (!protocol_message_format_send(my_pid, topic, line, message_text,
                                      sizeof(message_text))) {
      continue;
    }

    if (!queue_send(msqid, QUEUE_MESSAGE_TYPE_CONTROL, message_text)) {
      if (errno == EIDRM || errno == EINVAL) {
        view_queue_gone();
        success = false;
        break;
      }
    }
  }

  free(line);

  return success;
}