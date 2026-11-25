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

typedef struct {
  long pid;
  mqd_t queue;
} PeerEntry;

typedef struct {
  PeerEntry peers[CHAT_MAX_PEERS];
  int count;
} PeerTable;

static const char *peer_name_for(const PeerTable *table, long pid, char *name,
                                 size_t name_size) {
  for (int i = 0; i < table->count; ++i) {
    if (table->peers[i].pid == pid) {
      protocol_peer_name(name, name_size, i + 1);
      return name;
    }
  }

  return nullptr;
}

static void owner_broadcast(const PeerTable *table, const char *text,
                            unsigned int priority) {
  for (int i = 0; i < table->count; ++i) {
    chat_send_text(table->peers[i].queue, text, priority);
  }
}

static bool owner_handle_frame(MqChatQueues *queues, PeerTable *table,
                               char *buffer, unsigned int priority) {
  long pid = 0;
  char name[PROTOCOL_PEER_NAME_MAX];

  if (priority == CHAT_QUIT_PRIORITY) {
    if (!protocol_parse_bye_peer(buffer, &pid)) {
      return false;
    }

    if (peer_name_for(table, pid, name, sizeof(name)) == nullptr) {
      return true;
    }

    view_peer_left(name);

    for (int i = 0; i < table->count; ++i) {
      if (table->peers[i].pid == pid) {
        if (table->peers[i].queue != queues->send_queue) {
          mq_close(table->peers[i].queue);
        }

        table->peers[i] = table->peers[table->count - 1];
        --table->count;
        break;
      }
    }

    return true;
  }

  if (protocol_parse_hi(buffer, &pid)) {
    if (table->count >= (int)CHAT_MAX_PEERS) {
      fprintf(stderr, "Peer limit reached, pid %ld ignored\n", pid);
      return true;
    }

    const mqd_t private_queue = mq_peer_queue_open(queues->base_name, pid);

    if (private_queue == (mqd_t)-1) {
      fprintf(stderr, "Cannot open reply queue for pid %ld\n", pid);
      return true;
    }

    const int number = table->count + 1;

    char you[CHAT_MESSAGE_MAX];

    if (protocol_format_you(you, sizeof(you), pid, number)) {
      chat_send_text(private_queue, you, 0);
    }

    table->peers[table->count].pid = pid;
    table->peers[table->count].queue =
        number == 1 ? queues->send_queue : private_queue;

    if (number == 1) {
      mq_close(private_queue);
    }

    ++table->count;
    return true;
  }

  const char *text = nullptr;
  if (protocol_parse_msg_peer(buffer, &pid, &text)) {
    if (peer_name_for(table, pid, name, sizeof(name)) != nullptr) {
      view_message(name, text);
    }

    return true;
  }

  return false;
}

static void owner_drain_queue(MqChatQueues *queues, PeerTable *table) {
  char buffer[CHAT_MESSAGE_MAX];

  for (;;) {
    unsigned int priority = 0;
    const ssize_t size =
        mq_receive(queues->receive_queue, buffer, sizeof(buffer), &priority);

    if (size == -1) {
      if (errno != EAGAIN) {
        fprintf(stderr, "mq_receive: %s\n", strerror(errno));
      }

      return;
    }

    buffer[size - 1] = '\0';
    owner_handle_frame(queues, table, buffer, priority);
  }
}

bool chat_owner_run(MqChatQueues *queues) {
  PeerTable table = {.count = 0};
  char buffer[CHAT_MESSAGE_MAX];

  view_connected("owner");

  for (;;) {
    struct pollfd fds[2] = {
        {.fd = STDIN_FILENO, .events = POLLIN, .revents = 0},
        {.fd = (int)queues->receive_queue, .events = POLLIN, .revents = 0},
    };

    const int ready = poll(fds, 2, -1);

    if (ready == -1 && errno != EINTR) {
      fprintf(stderr, "poll: %s\n", strerror(errno));
      break;
    }

    if (ready > 0 && fds[1].revents & POLLIN) {
      owner_drain_queue(queues, &table);
    }

    if (signal_received != 0) {
      break;
    }

    if (ready > 0 && fds[0].revents & (POLLIN | POLLHUP)) {
      if (fgets(buffer, sizeof(buffer), stdin) == nullptr) {
        break;
      }

      buffer[strcspn(buffer, "\n")] = '\0';

      char message[CHAT_MESSAGE_MAX];

      if (protocol_format_msg_owner(message, sizeof(message), buffer)) {
        owner_broadcast(&table, message, 0);
      }
    }
  }

  char bye[CHAT_MESSAGE_MAX];

  if (protocol_format_bye_owner(bye, sizeof(bye))) {
    owner_broadcast(&table, bye, CHAT_QUIT_PRIORITY);
  }

  for (int i = 0; i < table.count; ++i) {
    if (table.peers[i].queue != queues->send_queue) {
      mq_close(table.peers[i].queue);
    }
  }

  return true;
}
