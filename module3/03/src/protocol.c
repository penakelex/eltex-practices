#define _POSIX_C_SOURCE 202405L

#include "protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void protocol_peer_name(char *peer_name, size_t size, int peer_number) {
  snprintf(peer_name, size, "peer%d", peer_number);
}

static bool format_prefix_pid(char *buffer, size_t size, const char *prefix,
                              long pid, const char *suffix) {
  const int written =
      snprintf(buffer, size, "%s%ld%s", prefix, pid, suffix ? suffix : "");
  return written > 0 && (size_t)written < size;
}

bool protocol_format_hi(char *buffer, size_t size, long pid) {
  return format_prefix_pid(buffer, size, "hi,", pid, "");
}

bool protocol_format_msg_peer(char *buffer, size_t size, long pid,
                              const char *text) {
  const int written = snprintf(buffer, size, "msg,%ld,%s", pid, text);
  return written > 0 && (size_t)written < size;
}

bool protocol_format_bye_peer(char *buffer, size_t size, long pid) {
  return format_prefix_pid(buffer, size, "bye,", pid, "");
}

bool protocol_format_you(char *buffer, size_t size, long pid, int peer_number) {
  const int written =
      snprintf(buffer, size, "you,%ld,peer%d", pid, peer_number);
  return written > 0 && (size_t)written < size;
}

bool protocol_format_msg_owner(char *buffer, size_t size, const char *text) {
  const int written = snprintf(buffer, size, "msg,%s", text);
  return written > 0 && (size_t)written < size;
}

bool protocol_format_bye_owner(char *buffer, size_t size) {
  const int written = snprintf(buffer, size, "bye");
  return written > 0 && (size_t)written < size;
}

static bool parse_prefix_pid(char *buffer, const char *prefix, long *pid) {
  const size_t prefix_length = strlen(prefix);
  if (strncmp(buffer, prefix, prefix_length) != 0) {
    return false;
  }

  char *end = nullptr;
  const long value = strtol(buffer + prefix_length, &end, 10);
  if (end == buffer + prefix_length || *end != '\0') {
    return false;
  }

  *pid = value;
  return true;
}

bool protocol_parse_hi(char *buffer, long *pid) {
  return parse_prefix_pid(buffer, "hi,", pid);
}

bool protocol_parse_msg_peer(char *buffer, long *pid, const char **text) {
  if (strncmp(buffer, "msg,", 4) != 0) {
    return false;
  }

  char *comma = strchr(buffer + 4, ',');
  if (comma == nullptr) {
    return false;
  }

  char *end = nullptr;
  const long value = strtol(buffer + 4, &end, 10);
  if (end != comma) {
    return false;
  }

  *pid = value;
  *text = comma + 1;
  return true;
}

bool protocol_parse_bye_peer(char *buffer, long *pid) {
  return parse_prefix_pid(buffer, "bye,", pid);
}

bool protocol_parse_you(char *buffer, long pid, int *peer_number,
                        char *peer_name, size_t peer_name_size) {
  if (strncmp(buffer, "you,", 4) != 0) {
    return false;
  }

  char *comma = strchr(buffer + 4, ',');
  if (comma == nullptr) {
    return false;
  }

  char *end = nullptr;
  const long value = strtol(buffer + 4, &end, 10);
  if (end != comma) {
    return false;
  }

  if (value != pid) {
    return false;
  }

  if (strncmp(comma + 1, "peer", 4) != 0) {
    return false;
  }

  const int number = (int)strtol(comma + 5, &end, 10);
  if (end == comma + 5 || *end != '\0') {
    return false;
  }

  *peer_number = number;
  protocol_peer_name(peer_name, peer_name_size, number);
  return true;
}

bool protocol_parse_msg_owner(char *buffer, const char **text) {
  if (strncmp(buffer, "msg,", 4) != 0) {
    return false;
  }

  *text = buffer + 4;
  return true;
}
