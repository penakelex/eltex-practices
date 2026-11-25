#define _POSIX_C_SOURCE 202405L

#include "chat_server.h"

#include <sys/epoll.h>
#include <unistd.h>

#include "client_registry.h"
#include "protocol.h"
#include "signal_state.h"
#include "tcp_socket.h"
#include "view.h"

constexpr int CHAT_SERVER_BACKLOG = 16;
constexpr int CHAT_SERVER_MAX_EVENTS = 64;
constexpr int CHAT_SERVER_POLL_TIMEOUT_MS = 200;

static void drop_client(const int epoll_fd, ClientRegistry *registry,
                        const int fd) {
  epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
  client_registry_remove(registry, fd);
  close(fd);
}

static void handle_new_client(const int epoll_fd, const int listen_fd,
                              ClientRegistry *registry) {
  const int client_fd = tcp_socket_accept(listen_fd);

  if (client_fd < 0) {
    return;
  }

  struct epoll_event event = {.events = EPOLLIN, .data.fd = client_fd};

  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0 ||
      !client_registry_add(registry, client_fd)) {
    close(client_fd);
    return;
  }
}

static void handle_client_data(const int epoll_fd, ClientRegistry *registry,
                               const int fd) {
  uint8_t buffer[CHAT_PACKET_BUFFER_SIZE];
  size_t size = 0;

  if (!tcp_socket_receive_packet(fd, buffer, sizeof(buffer), &size)) {
    drop_client(epoll_fd, registry, fd);
    return;
  }

  client_registry_broadcast(registry, fd, buffer, size);
}

int chat_server_run(const uint16_t port) {
  signal_state_install_handlers();

  const int listen_fd = tcp_socket_create();

  if (listen_fd < 0) {
    view_error("failed to create listen socket");
    return 1;
  }

  if (!tcp_socket_set_reuse(listen_fd) ||
      !tcp_socket_bind_listen(listen_fd, port, CHAT_SERVER_BACKLOG)) {
    view_error("failed to bind listen socket");
    close(listen_fd);
    return 1;
  }

  const int epoll_fd = epoll_create1(0);

  if (epoll_fd < 0) {
    view_error("failed to create epoll instance");
    close(listen_fd);
    return 1;
  }

  struct epoll_event listen_event = {.events = EPOLLIN, .data.fd = listen_fd};

  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &listen_event) < 0) {
    view_error("failed to watch listen socket");
    close(epoll_fd);
    close(listen_fd);
    return 1;
  }

  ClientRegistry registry;
  client_registry_initialize(&registry);
  struct epoll_event events[CHAT_SERVER_MAX_EVENTS];

  while (signal_received == 0) {
    const int ready = epoll_wait(epoll_fd, events, CHAT_SERVER_MAX_EVENTS,
                                 CHAT_SERVER_POLL_TIMEOUT_MS);

    if (ready < 0) {
      break;
    }

    for (int index = 0; index < ready; index += 1) {
      const int fd = events[index].data.fd;

      if (fd == listen_fd) {
        handle_new_client(epoll_fd, listen_fd, &registry);
        continue;
      }

      if ((events[index].events & (EPOLLIN | EPOLLHUP | EPOLLERR)) != 0) {
        handle_client_data(epoll_fd, &registry, fd);
      }
    }
  }

  for (size_t index = 0; index < client_registry_size(&registry); index += 1) {
    close(registry.fds[index]);
  }

  client_registry_free(&registry);
  close(epoll_fd);
  close(listen_fd);
  return 0;
}
