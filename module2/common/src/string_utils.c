#include <stdlib.h>
#include <string.h>
#include <string_utils.h>

char *duplicate_string(const char *source) {
  if (source == nullptr) {
    return nullptr;
  }

  const auto length = strlen(source);
  char *destination = malloc(length + 1);

  if (destination != nullptr) {
    memcpy(destination, source, length + 1);
  }

  return destination;
}