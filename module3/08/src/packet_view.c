#define _POSIX_C_SOURCE 202405L

#include "packet_view.h"

#include <arpa/inet.h>
#include <string.h>

static constexpr size_t VIEW_ADDRESS_TEXT = 16;

static bool mac_is_unknown(const uint8_t mac[PACKET_MAC_LENGTH]) {
  for (size_t index = 0; index < PACKET_MAC_LENGTH; index++) {
    if (mac[index] != 0) {
      return false;
    }
  }

  return true;
}

static void format_mac(const uint8_t mac[PACKET_MAC_LENGTH], char *out,
                       const size_t capacity) {
  if (mac_is_unknown(mac)) {
    snprintf(out, capacity, "??");
    return;
  }

  snprintf(out, capacity, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1],
           mac[2], mac[3], mac[4], mac[5]);
}

bool packet_view_format(const PacketInfo *packet, char *out,
                        const size_t capacity) {
  if (packet == nullptr || out == nullptr || capacity == 0) {
    return false;
  }

  char source_mac[18] = {0};
  char destination_mac[18] = {0};
  char source_address[VIEW_ADDRESS_TEXT] = {0};
  char destination_address[VIEW_ADDRESS_TEXT] = {0};

  format_mac(packet->source_mac, source_mac, sizeof(source_mac));
  format_mac(packet->destination_mac, destination_mac, sizeof(destination_mac));

  const struct in_addr source = {.s_addr = packet->source_address};
  const struct in_addr destination = {.s_addr = packet->destination_address};

  if (inet_ntop(AF_INET, &source, source_address, sizeof(source_address)) ==
          nullptr ||
      inet_ntop(AF_INET, &destination, destination_address,
                sizeof(destination_address)) == nullptr) {
    return false;
  }

  const int written = snprintf(
      out, capacity, "+%9.3fs MAC %s -> %s IP %s:%u -> %s:%u len=%zu%s%s",
      packet->timestamp, source_mac, destination_mac, source_address,
      packet->source_port, destination_address, packet->destination_port,
      packet->payload_length, packet->dns_name[0] != '\0' ? " dns=" : "",
      packet->dns_name);

  return written > 0 && (size_t)written < capacity;
}

void packet_view_print(const PacketList *list, FILE *output) {
  if (list == nullptr || output == nullptr) {
    return;
  }

  fprintf(output, "captured %zu segment(s)\n", list->count);

  for (size_t index = 0; index < list->count; index++) {
    char line[512] = {0};

    if (packet_view_format(&list->items[index], line, sizeof(line))) {
      fprintf(output, "%s\n", line);
    }
  }
}

void packet_view_usage(const char *program) {
  fprintf(stderr,
          "usage: %s -f <chat|dns|dhcp|ntp|mdns|quic|all>[,...] "
          "[-p chat_port] [-o output_file]\n",
          program);
}

void packet_view_error(const char *message) {
  fprintf(stderr, "error: %s\n", message);
}
