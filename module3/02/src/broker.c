#define _POSIX_C_SOURCE 202405L

#include "broker.h"
#include "protocol.h"
#include "queue.h"
#include "signal_state.h"
#include "view.h"

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>

typedef struct {
  pid_t pid;
  char topic[PROTOCOL_TOPIC_MAX];
} Subscription;

typedef struct {
  pid_t *items;
  size_t count;
  size_t capacity;
} PidList;

typedef struct {
  Subscription *items;
  size_t count;
  size_t capacity;
} SubscriptionList;

typedef struct {
  PidList publishers;
  PidList subscribers;
  SubscriptionList subscriptions;
} BrokerState;

static bool pid_list_add(PidList *list, const pid_t pid) {
  for (size_t i = 0; i < list->count; i++) {
    if (list->items[i] == pid) {
      return true;
    }
  }

  if (list->count == list->capacity) {
    const auto new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
    auto new_items =
        (pid_t *)realloc(list->items, new_capacity * sizeof(pid_t));

    if (new_items == nullptr) {
      return false;
    }

    list->items = new_items;
    list->capacity = new_capacity;
  }

  list->items[list->count++] = pid;

  return true;
}

static void pid_list_free(PidList *list) {
  free(list->items);
  list->items = nullptr;
  list->count = 0;
  list->capacity = 0;
}

static bool subscription_list_add(SubscriptionList *list, const pid_t pid,
                                  const char *topic) {
  for (size_t i = 0; i < list->count; i++) {
    if (list->items[i].pid == pid && strcmp(list->items[i].topic, topic) == 0) {
      return true;
    }
  }

  if (list->count == list->capacity) {
    const auto new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
    auto new_items = (Subscription *)realloc(
        list->items, new_capacity * sizeof(Subscription));

    if (new_items == nullptr) {
      return false;
    }

    list->items = new_items;
    list->capacity = new_capacity;
  }

  auto entry = &list->items[list->count];
  entry->pid = pid;
  strncpy(entry->topic, topic, PROTOCOL_TOPIC_MAX - 1);
  entry->topic[PROTOCOL_TOPIC_MAX - 1] = '\0';
  list->count++;

  return true;
}

static void subscription_list_remove(SubscriptionList *list, const pid_t pid,
                                     const char *topic) {
  for (size_t i = 0; i < list->count; i++) {
    if (list->items[i].pid == pid && strcmp(list->items[i].topic, topic) == 0) {
      list->items[i] = list->items[list->count - 1];
      list->count--;
      return;
    }
  }
}

static void subscription_list_free(SubscriptionList *list) {
  free(list->items);
  list->items = nullptr;
  list->count = 0;
  list->capacity = 0;
}

static void broker_handle_send(const int msqid, BrokerState *state,
                               const ProtocolMessage *parsed) {
  pid_list_add(&state->publishers, parsed->sender_pid);

  char delivery[QUEUE_MESSAGE_MAX_TEXT];

  if (!protocol_message_format_delivery(parsed->topic, parsed->payload,
                                        delivery, sizeof(delivery))) {
    return;
  }

  for (size_t i = 0; i < state->subscriptions.count; i++) {
    if (strcmp(state->subscriptions.items[i].topic, parsed->topic) == 0) {
      queue_send(msqid, (long)state->subscriptions.items[i].pid, delivery);
    }
  }
}

static void broker_handle_subscribe(BrokerState *state,
                                    const ProtocolMessage *parsed) {
  pid_list_add(&state->subscribers, parsed->sender_pid);
  subscription_list_add(&state->subscriptions, parsed->sender_pid,
                        parsed->topic);
}

static void broker_handle_unsubscribe(BrokerState *state,
                                      const ProtocolMessage *parsed) {
  subscription_list_remove(&state->subscriptions, parsed->sender_pid,
                           parsed->topic);
}

static void broker_dispatch(BrokerState *state, const int msqid,
                            const QueueMessage *message) {
  ProtocolMessage parsed;

  if (!protocol_message_parse(message->text, &parsed)) {
    return;
  }

  switch (parsed.kind) {
  case ProtocolMessageKindSend:
    broker_handle_send(msqid, state, &parsed);
    break;
  case ProtocolMessageKindSubscribe:
    broker_handle_subscribe(state, &parsed);
    break;
  case ProtocolMessageKindUnsubscribe:
    broker_handle_unsubscribe(state, &parsed);
    break;
  case ProtocolMessageKindUnknown:
    break;
  }
}

static void broker_notify_shutdown(const BrokerState *state) {
  for (size_t i = 0; i < state->publishers.count; i++) {
    kill(state->publishers.items[i], SIGINT);
  }

  for (size_t i = 0; i < state->subscribers.count; i++) {
    kill(state->subscribers.items[i], SIGINT);
  }
}

bool broker_run() {
  signal_state_install_handlers();
  signal_state_reset();

  const auto msqid = queue_create();

  if (msqid < 0) {
    view_queue_exists();
    return false;
  }

  view_broker_started();

  BrokerState state = {0};
  bool success = true;

  while (!signal_received) {
    QueueMessage message;
    const auto received =
        queue_receive(msqid, QUEUE_MESSAGE_TYPE_CONTROL, &message);

    if (received < 0) {
      if (errno == EINTR) {
        continue;
      }

      if (errno == EIDRM || errno == EINVAL) {
        success = false;
        break;
      }

      continue;
    }

    broker_dispatch(&state, msqid, &message);
  }

  broker_notify_shutdown(&state);
  queue_remove(msqid);

  pid_list_free(&state.publishers);
  pid_list_free(&state.subscribers);
  subscription_list_free(&state.subscriptions);

  view_broker_stopped();

  return success;
}