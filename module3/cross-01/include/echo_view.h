#pragma once

#include <stddef.h>
#include <stdint.h>

void echo_view_show_error(const char *message);
void echo_view_show_errno(const char *action);
void echo_view_show_usage(const char *program_name);
void echo_view_show_reply(const uint8_t *payload, size_t length);
void echo_view_show_request(uint32_t address, uint16_t port,
                            const uint8_t *payload, size_t length);
void echo_view_show_client_reset(uint32_t address, uint16_t port);
