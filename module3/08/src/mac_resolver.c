#define _POSIX_C_SOURCE 202405L
#define _DEFAULT_SOURCE

#include "mac_resolver.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static constexpr size_t ARP_TABLE_CAPACITY = 65536;

static bool mac_is_broadcast(const uint8_t out[6]) {
  return out[0] == 0xFF && out[1] == 0xFF && out[2] == 0xFF && out[3] == 0xFF &&
         out[4] == 0xFF && out[5] == 0xFF;
}

bool mac_parse(const char *text, uint8_t out[6]) {
  if (text == nullptr || out == nullptr) {
    return false;
  }

  unsigned int values[6] = {0};
  char trailing = '\0';

  if (sscanf(text, "%x:%x:%x:%x:%x:%x%c", &values[0], &values[1], &values[2],
             &values[3], &values[4], &values[5], &trailing) != 6) {
    return false;
  }

  for (size_t index = 0; index < 6; index++) {
    if (values[index] > 0xFF) {
      return false;
    }

    out[index] = (uint8_t)values[index];
  }

  return true;
}

bool mac_arp_lookup(const char *table, const size_t length,
                    const uint32_t address, uint8_t out[6]) {
  if (table == nullptr || out == nullptr) {
    return false;
  }

  char *copy = malloc(length + 1);

  if (copy == nullptr) {
    return false;
  }

  memcpy(copy, table, length);
  copy[length] = '\0';

  bool found = false;

  for (char *line = strtok(copy, "\n"); line != nullptr && !found;
       line = strtok(nullptr, "\n")) {
    char ip_text[16] = {0};
    char mac_text[18] = {0};

    if (sscanf(line, "%15s %*s %*s %17s", ip_text, mac_text) != 2) {
      continue;
    }

    struct in_addr ip = {0};

    if (inet_pton(AF_INET, ip_text, &ip) != 1) {
      continue;
    }

    if (ip.s_addr != address) {
      continue;
    }

    found = mac_parse(mac_text, out);
  }

  free(copy);
  return found;
}

static bool mac_resolve_local(const uint32_t address, uint8_t out[6]) {
  struct ifaddrs *interfaces = nullptr;

  if (getifaddrs(&interfaces) < 0) {
    return false;
  }

  char name[IF_NAMESIZE] = {0};
  bool found = false;

  for (const struct ifaddrs *entry = interfaces; entry != nullptr;
       entry = entry->ifa_next) {
    if (entry->ifa_addr == nullptr || entry->ifa_addr->sa_family != AF_INET) {
      continue;
    }

    const struct sockaddr_in *inet =
        (const struct sockaddr_in *)entry->ifa_addr;

    if (inet->sin_addr.s_addr == address) {
      found = true;
      snprintf(name, sizeof(name), "%s", entry->ifa_name);
      break;
    }

    if (entry->ifa_broadaddr != nullptr &&
        entry->ifa_broadaddr->sa_family == AF_INET) {
      const struct sockaddr_in *broadcast =
          (const struct sockaddr_in *)entry->ifa_broadaddr;

      if (broadcast->sin_addr.s_addr == address) {
        memset(out, 0xFF, 6);
        found = true;
        break;
      }
    }
  }

  if (found && !mac_is_broadcast(out)) {
    found = false;

    for (const struct ifaddrs *entry = interfaces; entry != nullptr;
         entry = entry->ifa_next) {
      if (entry->ifa_addr == nullptr ||
          entry->ifa_addr->sa_family != AF_PACKET) {
        continue;
      }

      if (strcmp(entry->ifa_name, name) != 0) {
        continue;
      }

      const struct sockaddr_ll *link =
          (const struct sockaddr_ll *)entry->ifa_addr;

      if (link->sll_halen == 6) {
        memcpy(out, link->sll_addr, 6);
        found = true;
      }

      break;
    }
  }

  freeifaddrs(interfaces);
  return found;
}

static bool mac_resolve_arp(const uint32_t address, uint8_t out[6]) {
  FILE *file = fopen("/proc/net/arp", "r");

  if (file == nullptr) {
    return false;
  }

  char *table = malloc(ARP_TABLE_CAPACITY);

  if (table == nullptr) {
    fclose(file);
    return false;
  }

  const size_t length = fread(table, 1, ARP_TABLE_CAPACITY - 1, file);
  fclose(file);

  const bool found = mac_arp_lookup(table, length, address, out);
  free(table);
  return found;
}

bool mac_resolve(const uint32_t address, uint8_t out[6]) {
  if (out == nullptr) {
    return false;
  }

  memset(out, 0, 6);

  if (address == htonl(INADDR_BROADCAST)) {
    memset(out, 0xFF, 6);
    return true;
  }

  if (mac_resolve_local(address, out)) {
    return true;
  }

  return mac_resolve_arp(address, out);
}
