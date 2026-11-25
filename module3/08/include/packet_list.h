#pragma once

#include "packet_parser.h"

typedef struct {
  PacketInfo *items;
  size_t count;
  size_t capacity;
} PacketList;

void packet_list_initialize(PacketList *list);
void packet_list_free(PacketList *list);
bool packet_list_add(PacketList *list, const PacketInfo *packet);
