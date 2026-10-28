#pragma once

#include <stddef.h>
#include <stdint.h>

constexpr size_t INPUT_LINE_BUFFER_SIZE = 256;

void read_line(char *buffer, size_t buffer_size);
void prompt_for_input(const char *message, char *buffer, size_t buffer_size);
uint32_t ask_for_uint32(const char *message, uint32_t minimum,
                        uint32_t maximum);