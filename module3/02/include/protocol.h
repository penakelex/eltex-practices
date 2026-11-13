#pragma once

#include <stddef.h>
#include <sys/types.h>

[[maybe_unused]] constexpr size_t PROTOCOL_TOPIC_MAX = 128;
[[maybe_unused]] constexpr size_t PROTOCOL_PAYLOAD_MAX = 3800;

typedef enum {
  ProtocolMessageKindSend,
  ProtocolMessageKindSubscribe,
  ProtocolMessageKindUnsubscribe,
  ProtocolMessageKindUnknown
} ProtocolMessageKind;

typedef struct {
  ProtocolMessageKind kind;
  pid_t sender_pid;
  char topic[PROTOCOL_TOPIC_MAX];
  char payload[PROTOCOL_PAYLOAD_MAX];
} ProtocolMessage;

bool protocol_message_parse(const char *text, ProtocolMessage *out);

bool protocol_message_format_send(pid_t sender_pid, const char *topic,
                                  const char *payload, char *out,
                                  size_t out_capacity);

bool protocol_message_format_subscribe(pid_t sender_pid, const char *topic,
                                       char *out, size_t out_capacity);

bool protocol_message_format_unsubscribe(pid_t sender_pid, const char *topic,
                                         char *out, size_t out_capacity);

bool protocol_message_format_delivery(const char *topic, const char *payload,
                                      char *out, size_t out_capacity);

bool protocol_message_format_delivery_parse(const char *text, char *topic_out,
                                            size_t topic_capacity,
                                            char *payload_out,
                                            size_t payload_capacity);