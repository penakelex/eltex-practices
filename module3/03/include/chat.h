#pragma once

#include <stddef.h>

[[maybe_unused]] constexpr unsigned int CHAT_QUIT_PRIORITY = 8;
[[maybe_unused]] constexpr size_t CHAT_MESSAGE_MAX = 8192;
[[maybe_unused]] constexpr size_t CHAT_MAX_PEERS = 32;

bool chat_run(const char *queue_name);
