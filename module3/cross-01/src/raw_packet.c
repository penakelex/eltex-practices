#define _POSIX_C_SOURCE 202405L

#include "raw_packet.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

static constexpr size_t IPV4_HEADER_LENGTH = 20;
static constexpr size_t UDP_HEADER_LENGTH = 8;
static constexpr uint8_t IPV4_VERSION_IHL = 0x45;
static constexpr uint8_t IPV4_TTL = 64;

static uint32_t checksum_accumulate(const void *data, size_t length,
                                    uint32_t sum) {
  const uint16_t *words = data;

  while (length > 1) {
    sum += *words++;
    length -= 2;
  }

  if (length != 0) {
    sum += *(const uint8_t *)words;
  }

  return sum;
}

static uint16_t checksum_finish(uint32_t sum) {
  while (sum >> 16 != 0) {
    sum = (sum & 0xffffu) + (sum >> 16);
  }

  return (uint16_t)~sum;
}

uint16_t raw_packet_checksum(const void *data, size_t length) {
  if (data == nullptr) {
    return 0;
  }

  return checksum_finish(checksum_accumulate(data, length, 0));
}

static void build_ipv4_header(uint8_t *buffer, size_t total_length,
                              uint32_t source_address,
                              uint32_t destination_address) {
  memset(buffer, 0, IPV4_HEADER_LENGTH);
  buffer[0] = IPV4_VERSION_IHL;

  const uint16_t network_total_length = htons((uint16_t)total_length);
  memcpy(buffer + 2, &network_total_length, sizeof(network_total_length));

  buffer[8] = IPV4_TTL;
  buffer[9] = IPPROTO_UDP;
  memcpy(buffer + 12, &source_address, sizeof(source_address));
  memcpy(buffer + 16, &destination_address, sizeof(destination_address));

  const uint16_t checksum = raw_packet_checksum(buffer, IPV4_HEADER_LENGTH);
  memcpy(buffer + 10, &checksum, sizeof(checksum));
}

static uint16_t udp_checksum(const UdpPacket *packet, const uint8_t *udp,
                             size_t udp_length) {
  uint32_t sum = checksum_accumulate(&packet->source_address,
                                     sizeof(packet->source_address), 0);
  sum = checksum_accumulate(&packet->destination_address,
                            sizeof(packet->destination_address), sum);

  const uint8_t zero_protocol[2] = {0, IPPROTO_UDP};
  sum = checksum_accumulate(zero_protocol, sizeof(zero_protocol), sum);

  const uint16_t network_udp_length = htons((uint16_t)udp_length);
  sum =
      checksum_accumulate(&network_udp_length, sizeof(network_udp_length), sum);
  sum = checksum_accumulate(udp, udp_length, sum);

  uint16_t checksum = checksum_finish(sum);

  if (checksum == 0) {
    checksum = 0xffffu;
  }

  return checksum;
}

size_t raw_packet_build(uint8_t *buffer, size_t capacity,
                        const UdpPacket *packet) {
  if (buffer == nullptr || packet == nullptr || packet->payload == nullptr) {
    return 0;
  }

  if (packet->payload_length >
      UINT16_MAX - IPV4_HEADER_LENGTH - UDP_HEADER_LENGTH) {
    return 0;
  }

  const size_t udp_length = UDP_HEADER_LENGTH + packet->payload_length;
  const size_t total_length = IPV4_HEADER_LENGTH + udp_length;

  if (capacity < total_length) {
    return 0;
  }

  build_ipv4_header(buffer, total_length, packet->source_address,
                    packet->destination_address);

  uint8_t *udp = buffer + IPV4_HEADER_LENGTH;
  const uint16_t source_port = htons(packet->source_port);
  const uint16_t destination_port = htons(packet->destination_port);
  const uint16_t network_udp_length = htons((uint16_t)udp_length);

  memcpy(udp, &source_port, sizeof(source_port));
  memcpy(udp + 2, &destination_port, sizeof(destination_port));
  memcpy(udp + 4, &network_udp_length, sizeof(network_udp_length));
  memset(udp + 6, 0, 2);
  memcpy(udp + UDP_HEADER_LENGTH, packet->payload, packet->payload_length);

  const uint16_t checksum = udp_checksum(packet, udp, udp_length);
  memcpy(udp + 6, &checksum, sizeof(checksum));

  return total_length;
}

bool raw_packet_parse(const uint8_t *buffer, size_t length, UdpPacket *packet) {
  if (buffer == nullptr || packet == nullptr) {
    return false;
  }

  if (length < IPV4_HEADER_LENGTH + UDP_HEADER_LENGTH) {
    return false;
  }

  if ((buffer[0] >> 4) != 4) {
    return false;
  }

  const size_t ip_header_length = (size_t)(buffer[0] & 0x0f) * 4;

  if (ip_header_length < IPV4_HEADER_LENGTH ||
      length < ip_header_length + UDP_HEADER_LENGTH) {
    return false;
  }

  if (buffer[9] != IPPROTO_UDP) {
    return false;
  }

  uint16_t total_length = 0;
  memcpy(&total_length, buffer + 2, sizeof(total_length));
  total_length = ntohs(total_length);

  if (total_length > length ||
      total_length < ip_header_length + UDP_HEADER_LENGTH) {
    return false;
  }

  const uint8_t *udp = buffer + ip_header_length;
  uint16_t udp_length = 0;
  memcpy(&udp_length, udp + 4, sizeof(udp_length));
  udp_length = ntohs(udp_length);

  if (udp_length < UDP_HEADER_LENGTH ||
      ip_header_length + udp_length > total_length) {
    return false;
  }

  memcpy(&packet->source_address, buffer + 12, sizeof(packet->source_address));
  memcpy(&packet->destination_address, buffer + 16,
         sizeof(packet->destination_address));

  uint16_t source_port = 0;
  uint16_t destination_port = 0;
  memcpy(&source_port, udp, sizeof(source_port));
  memcpy(&destination_port, udp + 2, sizeof(destination_port));
  packet->source_port = ntohs(source_port);
  packet->destination_port = ntohs(destination_port);
  packet->payload = udp + UDP_HEADER_LENGTH;
  packet->payload_length = udp_length - UDP_HEADER_LENGTH;

  return true;
}
