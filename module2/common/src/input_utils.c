#include <input_utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

constexpr size_t INTEGER_INPUT_BUFFER_SIZE = 32;

void read_line(char *buffer, const size_t buffer_size) {
  if (fgets(buffer, (int)buffer_size, stdin) == nullptr) {
    buffer[0] = '\0';
    return;
  }

  size_t length = strlen(buffer);

  while (length > 0 &&
         (buffer[length - 1] == '\n' || buffer[length - 1] == '\r')) {
    buffer[--length] = '\0';
  }
}

void prompt_for_input(const char *message, char *buffer,
                      const size_t buffer_size) {
  fputs(message, stdout);
  fflush(stdout);
  read_line(buffer, buffer_size);
}

uint32_t ask_for_uint32(const char *message, const uint32_t minimum,
                        const uint32_t maximum) {
  char buffer[INTEGER_INPUT_BUFFER_SIZE];

  while (true) {
    prompt_for_input(message, buffer, sizeof buffer);
    char *end_pointer = nullptr;
    const unsigned long value = strtoul(buffer, &end_pointer, 10);

    if (end_pointer != buffer && *end_pointer == '\0' && value >= minimum &&
        value <= maximum) {
      return (uint32_t)value;
    }

    puts("Некорректный ввод, попробуйте ещё раз.");
  }
}