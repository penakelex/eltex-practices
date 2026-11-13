#define _POSIX_C_SOURCE 202405L

#include "protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool protocol_message_format_send(pid_t sender_pid, const char *topic,
                                  const char *payload, char *out,
                                  size_t out_capacity) {
  if (sender_pid <= 0 || topic == nullptr || payload == nullptr ||
      out == nullptr) {
    return false;
  }

  const auto written = snprintf(out, out_capacity, "send,%d,%s\n%s",
                                (int)sender_pid, topic, payload);
  return written > 0 && (size_t)written < out_capacity;
}

bool protocol_message_format_subscribe(pid_t sender_pid, const char *topic,
                                       char *out, size_t out_capacity) {
  if (sender_pid <= 0 || topic == nullptr || out == nullptr) {
    return false;
  }

  const auto written =
      snprintf(out, out_capacity, "subscribe,%d,%s", (int)sender_pid, topic);
  return written > 0 && (size_t)written < out_capacity;
}

bool protocol_message_format_unsubscribe(pid_t sender_pid, const char *topic,
                                         char *out, size_t out_capacity) {
  if (sender_pid <= 0 || topic == nullptr || out == nullptr) {
    return false;
  }

  const auto written =
      snprintf(out, out_capacity, "unsubscribe,%d,%s", (int)sender_pid, topic);
  return written > 0 && (size_t)written < out_capacity;
}

bool protocol_message_format_delivery(const char *topic, const char *payload,
                                      char *out, size_t out_capacity) {
  if (topic == nullptr || payload == nullptr || out == nullptr) {
    return false;
  }

  const auto written = snprintf(out, out_capacity, "%s\n%s", topic, payload);
  return written > 0 && (size_t)written < out_capacity;
}

bool protocol_message_format_delivery_parse(const char *text, char *topic_out,
                                            size_t topic_capacity,
                                            char *payload_out,
                                            size_t payload_capacity) {
  if (text == nullptr || topic_out == nullptr || payload_out == nullptr) {
    return false;
  }

  const auto newline = strchr(text, '\n');

  if (newline == nullptr) {
    return false;
  }

  const auto topic_length = (size_t)(newline - text);

  if (topic_length == 0 || topic_length >= topic_capacity) {
    return false;
  }

  const auto payload_length = strlen(newline + 1);

  if (payload_length >= payload_capacity) {
    return false;
  }

  memcpy(topic_out, text, topic_length);
  topic_out[topic_length] = '\0';
  memcpy(payload_out, newline + 1, payload_length + 1);

  return true;
}

static bool parse_header(const char *text, size_t header_length,
                         ProtocolMessage *out) {
  const auto first_comma = (const char *)memchr(text, ',', header_length);

  if (first_comma == nullptr) {
    return false;
  }

  const auto first_comma_offset = (size_t)(first_comma - text);
  const auto second_comma = (const char *)memchr(
      first_comma + 1, ',', header_length - first_comma_offset - 1);

  if (second_comma == nullptr) {
    return false;
  }

  const auto kind_length = first_comma_offset;
  const auto pid_length = (size_t)(second_comma - first_comma) - 1;
  const auto topic_length = header_length - (size_t)(second_comma - text) - 1;

  if (topic_length == 0 || topic_length >= PROTOCOL_TOPIC_MAX) {
    return false;
  }

  ProtocolMessageKind kind = ProtocolMessageKindUnknown;

  if (kind_length == 4 && strncmp(text, "send", 4) == 0) {
    kind = ProtocolMessageKindSend;
  } else if (kind_length == 9 && strncmp(text, "subscribe", 9) == 0) {
    kind = ProtocolMessageKindSubscribe;
  } else if (kind_length == 11 && strncmp(text, "unsubscribe", 11) == 0) {
    kind = ProtocolMessageKindUnsubscribe;
  } else {
    return false;
  }

  char pid_buffer[32];

  if (pid_length >= sizeof(pid_buffer)) {
    return false;
  }

  memcpy(pid_buffer, first_comma + 1, pid_length);
  pid_buffer[pid_length] = '\0';

  char *end = nullptr;
  const long pid = strtol(pid_buffer, &end, 10);

  if (end == pid_buffer || *end != '\0' || pid <= 0) {
    return false;
  }

  out->kind = kind;
  out->sender_pid = (pid_t)pid;
  memcpy(out->topic, second_comma + 1, topic_length);
  out->topic[topic_length] = '\0';

  return true;
}

bool protocol_message_parse(const char *text, ProtocolMessage *out) {
  if (text == nullptr || out == nullptr) {
    return false;
  }

  memset(out, 0, sizeof(*out));
  out->kind = ProtocolMessageKindUnknown;

  const auto newline = strchr(text, '\n');
  const auto header_length =
      newline != nullptr ? (size_t)(newline - text) : strlen(text);

  if (!parse_header(text, header_length, out)) {
    return false;
  }

  if (newline != nullptr) {
    const auto payload_length = strlen(newline + 1);

    if (payload_length >= PROTOCOL_PAYLOAD_MAX) {
      return false;
    }

    memcpy(out->payload, newline + 1, payload_length + 1);
  }

  return true;
}