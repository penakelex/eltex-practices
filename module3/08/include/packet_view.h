#pragma once

#include <stdio.h>

#include "packet_list.h"

bool packet_view_format(const PacketInfo *packet, char *out,
                        const size_t capacity);
void packet_view_print(const PacketList *list, FILE *output);
void packet_view_usage(const char *program);
void packet_view_error(const char *message);
