#pragma once

#include <stddef.h>
#include <stdint.h>

bool mac_resolve(const uint32_t address, uint8_t out[6]);
bool mac_arp_lookup(const char *table, const size_t length,
                    const uint32_t address, uint8_t out[6]);
bool mac_parse(const char *text, uint8_t out[6]);
