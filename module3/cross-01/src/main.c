#define _POSIX_C_SOURCE 202405L

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "echo_client.h"
#include "echo_server.h"
#include "echo_view.h"

static constexpr uint16_t DEFAULT_PORT = 5000;

static volatile sig_atomic_t stop_requested = 0;

static void handle_stop(const int signal_number) {
  (void)signal_number;
  stop_requested = 1;
}

static bool install_stop_handler() {
  struct sigaction action = {0};
  action.sa_handler = handle_stop;
  sigemptyset(&action.sa_mask);
  return sigaction(SIGINT, &action, nullptr) == 0;
}

static bool parse_port(const char *value, uint16_t *out_port) {
  char *end = nullptr;
  const long port = strtol(value, &end, 10);

  if (end == value || *end != '\0' || port <= 0 || port > 65535) {
    return false;
  }

  *out_port = (uint16_t)port;
  return true;
}

static bool parse_arguments(const int argc, char *argv[], bool *server_mode,
                            uint32_t *server_address, uint16_t *port) {
  *server_mode = false;
  *server_address = 0;
  *port = DEFAULT_PORT;

  bool mode_given = false;

  for (int index = 1; index < argc; index++) {
    if (strcmp(argv[index], "-s") == 0) {
      if (mode_given) {
        return false;
      }

      *server_mode = true;
      mode_given = true;
      continue;
    }

    if (strcmp(argv[index], "-c") == 0 && index + 1 < argc) {
      if (mode_given) {
        return false;
      }

      if (inet_pton(AF_INET, argv[++index], server_address) != 1) {
        return false;
      }

      mode_given = true;
      continue;
    }

    if (strcmp(argv[index], "-p") == 0 && index + 1 < argc) {
      if (!parse_port(argv[++index], port)) {
        return false;
      }

      continue;
    }

    return false;
  }

  return mode_given;
}

int main(const int argc, char *argv[]) {
  bool server_mode = false;
  uint32_t server_address = 0;
  uint16_t port = 0;

  if (!parse_arguments(argc, argv, &server_mode, &server_address, &port)) {
    echo_view_show_usage(argv[0]);
    return EXIT_FAILURE;
  }

  const int socket_descriptor = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

  if (socket_descriptor < 0) {
    echo_view_show_error(errno == EPERM || errno == EACCES
                             ? "нет прав на raw-сокет, запустите под root"
                             : "не удалось создать raw-сокет");
    return EXIT_FAILURE;
  }

  const int header_included = 1;

  if (setsockopt(socket_descriptor, IPPROTO_IP, IP_HDRINCL, &header_included,
                 sizeof(header_included)) < 0) {
    echo_view_show_error("не удалось установить IP_HDRINCL");
    close(socket_descriptor);
    return EXIT_FAILURE;
  }

  if (!install_stop_handler()) {
    echo_view_show_error("не удалось установить обработчик SIGINT");
    close(socket_descriptor);
    return EXIT_FAILURE;
  }

  const bool ok =
      server_mode ? echo_server_run(socket_descriptor, port, &stop_requested)
                  : echo_client_run(socket_descriptor, server_address, port,
                                    &stop_requested);

  close(socket_descriptor);

  if (!ok) {
    echo_view_show_error("работа завершилась с ошибкой");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
