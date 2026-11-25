#define _POSIX_C_SOURCE 202405L

#include "echo_view.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static void format_address(uint32_t address, char *buffer, size_t capacity) {
  if (inet_ntop(AF_INET, &address, buffer, (socklen_t)capacity) == nullptr) {
    snprintf(buffer, capacity, "?");
  }
}

void echo_view_show_error(const char *message) {
  if (message == nullptr) {
    return;
  }

  fprintf(stderr, "Ошибка: %s\n", message);
}

void echo_view_show_errno(const char *action) {
  if (action == nullptr) {
    return;
  }

  fprintf(stderr, "Ошибка: %s: %s\n", action, strerror(errno));
}

void echo_view_show_usage(const char *program_name) {
  if (program_name == nullptr) {
    return;
  }

  fprintf(stderr, "Использование:\n");
  fprintf(stderr, "  %s -s [-p порт]           — режим сервера\n",
          program_name);
  fprintf(stderr, "  %s -c адрес [-p порт]     — режим клиента\n",
          program_name);
}

void echo_view_show_reply(const uint8_t *payload, size_t length) {
  if (payload == nullptr) {
    return;
  }

  printf("%.*s\n", (int)length, (const char *)payload);
  fflush(stdout);
}

void echo_view_show_request(uint32_t address, uint16_t port,
                            const uint8_t *payload, size_t length) {
  if (payload == nullptr) {
    return;
  }

  char text[INET_ADDRSTRLEN] = {0};
  format_address(address, text, sizeof(text));

  printf("Клиент %s:%u -> %.*s\n", text, port, (int)length,
         (const char *)payload);
  fflush(stdout);
}

void echo_view_show_client_reset(uint32_t address, uint16_t port) {
  char text[INET_ADDRSTRLEN] = {0};
  format_address(address, text, sizeof(text));

  printf("Клиент %s:%u закрыл соединение, счётчик сброшен\n", text, port);
  fflush(stdout);
}
