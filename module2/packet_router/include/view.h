#pragma once

#include "packet_router.h"

void view_print_usage(const char *program_name);
void view_print_error(PacketRouterStatus status);
void view_print_stats(const PacketStats *stats);