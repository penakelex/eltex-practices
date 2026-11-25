#define _POSIX_C_SOURCE 202405L

#include "chat_internal.h"
#include "protocol.h"
#include "signal_state.h"
#include "view.h"

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static constexpr int REGISTER_TIMEOUT_MS = 5000;

static bool peer_register(MqChatQueues *queues, long pid, char *peer_name,
                          size_t peer_name_size, int *assigned_number) {
  char buffer[CHAT_MESSAGE_MAX];

  if (!protocol_format_hi(buffer, sizeof(buffer), pid)) {
    return false;
  }

  if (!chat_send_text(queues->send_queue, buffer, 0)) {
    return false;
  }

  int waited_ms = 0;

  while (waited_ms < REGISTER_TIMEOUT_MS && signal_received == 0) {
    struct pollfd fd = {
        .fd = (int)queues->receive_queue, .events = POLLIN, .revents = 0};

    const int ready = poll(&fd, 1, 100);
    waited_ms += 100;

    if (ready == -1) {
      if (errno == EINTR) {
        continue;
      }

      fprintf(stderr, "poll: %s\n", strerror(errno));
      return false;
    }

    if (ready == 0 || !(fd.revents & POLLIN)) {
      continue;
    }

    unsigned int priority = 0;
    const ssize_t size =
        mq_receive(queues->receive_queue, buffer, sizeof(buffer), &priority);

    if (size == -1) {
      continue;
    }

    buffer[size - 1] = '\0';

    if (priority == CHAT_QUIT_PRIORITY) {
      view_owner_left();
      return false;
    }

    if (protocol_parse_you(buffer, pid, assigned_number, peer_name,
                           peer_name_size)) {
      return true;
    }
  }

  if (signal_received == 0) {
    view_no_owner(queues->first_name);
  }

  return false;
}

static bool peer_handle_frame(char *buffer, unsigned int priority) {
  if (priority == CHAT_QUIT_PRIORITY) {
    view_owner_left();
    return true;
  }

  const char *text = nullptr;
  if (protocol_parse_msg_owner(buffer, &text)) {
    view_message("owner", text);
  }

  return false;
}

static void peer_drain_queue(mqd_t queue, char *buffer, size_t buffer_size,
                             bool *owner_left) {
  for (;;) {
    unsigned int priority = 0;
    const ssize_t size = mq_receive(queue, buffer, buffer_size, &priority);
    if (size == -1) {
      break;
    }

    buffer[size - 1] = '\0';
    if (peer_handle_frame(buffer, priority)) {
      *owner_left = true;
    }
  }
}

bool chat_peer_run(MqChatQueues *queues, long pid) {
  char self_name[PROTOCOL_PEER_NAME_MAX];
  int assigned_number = 0;

  if (!peer_register(queues, pid, self_name, sizeof(self_name),
                     &assigned_number)) {
    return true;
  }

  view_connected(self_name);

  const bool is_first = assigned_number == 1;
  char buffer[CHAT_MESSAGE_MAX];
  bool owner_left = false;

  while (!owner_left) {
    struct pollfd fds[3] = {
        {.fd = STDIN_FILENO, .events = POLLIN, .revents = 0},
        {.fd = (int)queues->receive_queue, .events = POLLIN, .revents = 0},
        {.fd = (int)queues->second_queue, .events = POLLIN, .revents = 0},
    };
    const nfds_t fd_count = is_first ? 3 : 2;

    const int ready = poll(fds, fd_count, -1);

    if (ready == -1 && errno != EINTR) {
      fprintf(stderr, "poll: %s\n", strerror(errno));
      break;
    }

    if (ready > 0 && fds[1].revents & POLLIN) {
      peer_drain_queue(queues->receive_queue, buffer, sizeof(buffer),
                       &owner_left);
    }

    if (is_first && ready > 0 && fds[2].revents & POLLIN) {
      peer_drain_queue(queues->second_queue, buffer, sizeof(buffer),
                       &owner_left);
    }

    if (signal_received != 0 || owner_left) {
      break;
    }

    if (ready > 0 && fds[0].revents & (POLLIN | POLLHUP)) {
      if (fgets(buffer, sizeof(buffer), stdin) == nullptr) {
        break;
      }

      buffer[strcspn(buffer, "\n")] = '\0';

      char message[CHAT_MESSAGE_MAX];

      if (protocol_format_msg_peer(message, sizeof(message), pid, buffer)) {
        chat_send_text(queues->send_queue, message, 0);
      }
    }
  }

  char bye[CHAT_MESSAGE_MAX];

  if (protocol_format_bye_peer(bye, sizeof(bye), pid)) {
    chat_send_text(queues->send_queue, bye, CHAT_QUIT_PRIORITY);
  }

  return true;
}
