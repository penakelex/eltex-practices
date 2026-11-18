#pragma once

#include <stddef.h>

[[maybe_unused]] constexpr size_t PROTOCOL_PEER_NAME_MAX = 16;

bool protocol_format_hi(char *buffer, size_t size, long pid);

bool protocol_format_msg_peer(char *buffer, size_t size, long pid,
                              const char *text);

bool protocol_format_bye_peer(char *buffer, size_t size, long pid);

bool protocol_format_you(char *buffer, size_t size, long pid, int peer_number);

bool protocol_format_msg_owner(char *buffer, size_t size, const char *text);

bool protocol_format_bye_owner(char *buffer, size_t size);

bool protocol_parse_hi(char *buffer, long *pid);

bool protocol_parse_msg_peer(char *buffer, long *pid, const char **text);

bool protocol_parse_bye_peer(char *buffer, long *pid);

bool protocol_parse_you(char *buffer, long pid, int *peer_number,
                        char *peer_name, size_t peer_name_size);

bool protocol_parse_msg_owner(char *buffer, const char **text);

void protocol_peer_name(char *peer_name, size_t size, int peer_number);
