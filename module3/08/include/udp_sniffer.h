#pragma once

#include "packet_filter.h"
#include "packet_list.h"

int udp_sniffer_open();
bool udp_sniffer_capture(const int fd, const FilterKind filter,
                         const uint16_t chat_port, PacketList *out);
void udp_sniffer_close(const int fd);
