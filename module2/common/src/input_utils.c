#include <errno.h>
#include <input_utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

constexpr size_t INTEGER_INPUT_BUFFER_SIZE = 32;

bool read_line(char *buffer, const size_t buffer_size) {
  if (fgets(buffer, (int)buffer_size, stdin) == nullptr) {
    buffer[0] = '\0';
    return false;
  }

  size_t length = strlen(buffer);

  while (length > 0 &&
         (buffer[length - 1] == '\n' || buffer[length - 1] == '\r')) {
    buffer[--length] = '\0';
  }

  return true;
}

bool prompt_for_input(const char *message, char *buffer,
                      const size_t buffer_size) {
  fputs(message, stdout);
  fflush(stdout);
  return read_line(buffer, buffer_size);
}

bool parse_uint32(const char *input, uint32_t *out) {
  if (input == nullptr || out == nullptr) {
    return false;
  }

  if (input[0] == '\0') {
    return false;
  }

  for (const char *p = input; *p != '\0'; p++) {
    if (*p < '0' || *p > '9') {
      return false;
    }
  }

  errno = 0;
  char *end_pointer = nullptr;
  const unsigned long value = strtoul(input, &end_pointer, 10);

  if (*end_pointer != '\0' || errno == ERANGE || value > UINT32_MAX) {
    return false;
  }

  *out = (uint32_t)value;
  return true;
}

uint32_t ask_for_uint32(const char *message, const uint32_t minimum,
                        const uint32_t maximum) {
  char buffer[INTEGER_INPUT_BUFFER_SIZE];

  while (true) {
    prompt_for_input(message, buffer, sizeof buffer);
    uint32_t value = 0;

    if (parse_uint32(buffer, &value) && value >= minimum && value <= maximum) {
      return value;
    }

    puts("Некорректный ввод, попробуйте ещё раз.");
  }
}