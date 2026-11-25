#define _POSIX_C_SOURCE 202405L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "packet_view.h"
#include "udp_sniffer.h"

static constexpr uint16_t DEFAULT_CHAT_PORT = 4000;

static bool parse_port(const char *value, uint16_t *out) {
  char *end = nullptr;
  const long port = strtol(value, &end, 10);

  if (end == value || *end != '\0' || port <= 0 || port > 65535) {
    return false;
  }

  *out = (uint16_t)port;
  return true;
}

static bool parse_arguments(const int argc, char *argv[], FilterKind *filter,
                            uint16_t *chat_port, const char **output_file) {
  *filter = FILTER_CHAT;
  *chat_port = DEFAULT_CHAT_PORT;
  *output_file = nullptr;

  bool filter_given = false;

  for (int index = 1; index < argc; index++) {
    if (strcmp(argv[index], "-f") == 0 && index + 1 < argc) {
      filter_given = packet_filter_parse(argv[++index], filter);
      continue;
    }

    if (strcmp(argv[index], "-p") == 0 && index + 1 < argc) {
      if (!parse_port(argv[++index], chat_port)) {
        return false;
      }
      continue;
    }

    if (strcmp(argv[index], "-o") == 0 && index + 1 < argc) {
      *output_file = argv[++index];
      continue;
    }

    return false;
  }

  return filter_given;
}

int main(int argc, char *argv[]) {
  FilterKind filter = FILTER_CHAT;
  uint16_t chat_port = 0;
  const char *output_file = nullptr;

  if (!parse_arguments(argc, argv, &filter, &chat_port, &output_file)) {
    packet_view_usage(argv[0]);
    return 1;
  }

  const int fd = udp_sniffer_open();

  if (fd < 0) {
    packet_view_error(errno == EPERM || errno == EACCES
                          ? "raw socket requires root or CAP_NET_RAW"
                          : "failed to open raw socket");
    return 1;
  }

  PacketList packets;
  packet_list_initialize(&packets);

  const bool captured = udp_sniffer_capture(fd, filter, chat_port, &packets);
  udp_sniffer_close(fd);

  if (!captured) {
    packet_view_error("capture failed");
    packet_list_free(&packets);
    return 1;
  }

  packet_view_print(&packets, stdout);

  if (output_file != nullptr) {
    FILE *file = fopen(output_file, "w");

    if (file == nullptr) {
      packet_view_error("failed to open output file");
      packet_list_free(&packets);
      return 1;
    }

    packet_view_print(&packets, file);
    fclose(file);
  }

  packet_list_free(&packets);
  return 0;
}
