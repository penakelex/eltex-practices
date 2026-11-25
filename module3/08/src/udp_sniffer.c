#define _POSIX_C_SOURCE 202405L

#include "udp_sniffer.h"

#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "mac_resolver.h"

[[maybe_unused]] constexpr size_t SNIFFER_BUFFER_CAPACITY = 65536;

static volatile sig_atomic_t capture_stopped = 0;

static void sniffer_stop(const int signal_number) {
  (void)signal_number;
  capture_stopped = 1;
}

static double sniffer_elapsed(const struct timespec *start) {
  struct timespec now = {0};
  clock_gettime(CLOCK_MONOTONIC, &now);
  return (double)(now.tv_sec - start->tv_sec) +
         (double)(now.tv_nsec - start->tv_nsec) / 1e9;
}

static bool sniffer_install_handler() {
  struct sigaction action = {0};
  action.sa_handler = sniffer_stop;
  sigemptyset(&action.sa_mask);

  return sigaction(SIGINT, &action, nullptr) == 0;
}

int udp_sniffer_open() {
  return socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
}

bool udp_sniffer_capture(const int fd, const FilterKind filter,
                         const uint16_t chat_port, PacketList *out) {
  if (fd < 0 || out == nullptr) {
    return false;
  }

  if (!sniffer_install_handler()) {
    return false;
  }

  capture_stopped = 0;

  struct timespec start = {0};
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint8_t buffer[SNIFFER_BUFFER_CAPACITY];

  while (!capture_stopped) {
    const ssize_t received = recv(fd, buffer, sizeof(buffer), 0);

    if (received < 0) {
      if (errno == EINTR) {
        continue;
      }

      return false;
    }

    PacketInfo packet = {0};

    if (!packet_parse(buffer, (size_t)received, &packet)) {
      continue;
    }

    if (!packet_filter_match(filter, &packet, chat_port)) {
      continue;
    }

    packet.timestamp = sniffer_elapsed(&start);
    mac_resolve(packet.source_address, packet.source_mac);
    mac_resolve(packet.destination_address, packet.destination_mac);

    if (!packet_list_add(out, &packet)) {
      return false;
    }
  }

  return true;
}

void udp_sniffer_close(const int fd) {
  if (fd >= 0) {
    close(fd);
  }
}
