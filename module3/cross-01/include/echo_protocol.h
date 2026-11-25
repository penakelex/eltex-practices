#pragma once

#include <stddef.h>
#include <stdint.h>

[[maybe_unused]] constexpr size_t ECHO_MESSAGE_CAPACITY = 1024;

bool echo_protocol_is_close(const uint8_t *payload, size_t length);
size_t echo_protocol_build_close(uint8_t *buffer, size_t capacity);
