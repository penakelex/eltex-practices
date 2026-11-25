#pragma once

#include <stdint.h>

#include <netinet/in.h>

#include "protocol.h"

typedef struct {
  int fd;
  uint16_t port;
  char name[CHAT_NAME_CAPACITY];
  struct sockaddr_in broadcast_address;
} UdpChatState;

int udp_chat_loop(UdpChatState *state);
