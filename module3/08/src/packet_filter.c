#define _POSIX_C_SOURCE 202405L

#include "packet_filter.h"

#include <string.h>

static constexpr uint16_t DNS_PORT = 53;
static constexpr uint16_t DHCP_SERVER_PORT = 67;
static constexpr uint16_t DHCP_CLIENT_PORT = 68;
static constexpr uint16_t NTP_PORT = 123;
static constexpr uint16_t MDNS_PORT = 5353;
static constexpr uint16_t QUIC_PORT = 443;
static constexpr size_t FILTER_NAME_CAPACITY = 16;

static bool packet_filter_port_match(const PacketInfo *packet,
                                     const uint16_t first,
                                     const uint16_t second) {
  return packet->source_port == first || packet->destination_port == first ||
         packet->source_port == second || packet->destination_port == second;
}

static bool packet_filter_parse_single(const char *name, FilterKind *out) {
  if (strcmp(name, "chat") == 0) {
    *out = FILTER_CHAT;
    return true;
  }

  if (strcmp(name, "dns") == 0) {
    *out = FILTER_DNS;
    return true;
  }

  if (strcmp(name, "dhcp") == 0) {
    *out = FILTER_DHCP;
    return true;
  }

  if (strcmp(name, "ntp") == 0) {
    *out = FILTER_NTP;
    return true;
  }

  if (strcmp(name, "mdns") == 0) {
    *out = FILTER_MDNS;
    return true;
  }

  if (strcmp(name, "quic") == 0) {
    *out = FILTER_QUIC;
    return true;
  }

  if (strcmp(name, "all") == 0) {
    *out = FILTER_ALL;
    return true;
  }

  return false;
}

bool packet_filter_parse(const char *names, FilterKind *out) {
  if (names == nullptr || out == nullptr || names[0] == '\0') {
    return false;
  }

  FilterKind combined = FILTER_NONE;

  for (const char *begin = names; *begin != '\0';) {
    const char *comma = strchr(begin, ',');
    const size_t length =
        comma != nullptr ? (size_t)(comma - begin) : strlen(begin);

    if (length == 0 || length >= FILTER_NAME_CAPACITY) {
      return false;
    }

    char token[FILTER_NAME_CAPACITY] = {0};
    memcpy(token, begin, length);

    FilterKind single = FILTER_NONE;

    if (!packet_filter_parse_single(token, &single)) {
      return false;
    }

    combined = (FilterKind)(combined | single);

    if (comma == nullptr) {
      break;
    }

    begin = comma + 1;

    if (*begin == '\0') {
      return false;
    }
  }

  *out = combined;
  return true;
}

bool packet_filter_match(const FilterKind kind, const PacketInfo *packet,
                         const uint16_t chat_port) {
  if (packet == nullptr) {
    return false;
  }

  if ((kind & FILTER_CHAT) != 0 &&
      packet_filter_port_match(packet, chat_port, chat_port)) {
    return true;
  }

  if ((kind & FILTER_DNS) != 0 &&
      packet_filter_port_match(packet, DNS_PORT, DNS_PORT)) {
    return true;
  }

  if ((kind & FILTER_DHCP) != 0 &&
      packet_filter_port_match(packet, DHCP_SERVER_PORT, DHCP_CLIENT_PORT)) {
    return true;
  }

  if ((kind & FILTER_NTP) != 0 &&
      packet_filter_port_match(packet, NTP_PORT, NTP_PORT)) {
    return true;
  }

  if ((kind & FILTER_MDNS) != 0 &&
      packet_filter_port_match(packet, MDNS_PORT, MDNS_PORT)) {
    return true;
  }

  if ((kind & FILTER_QUIC) != 0 &&
      packet_filter_port_match(packet, QUIC_PORT, QUIC_PORT)) {
    return true;
  }

  return false;
}
