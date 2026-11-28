#define _POSIX_C_SOURCE 202405L

#include "taxi_controller.h"

#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>

#include "driver.h"
#include "driver_table.h"
#include "taxi_protocol.h"
#include "taxi_view.h"

[[maybe_unused]] constexpr size_t LINE_CAPACITY = 256;

static bool spawn_driver(DriverTable *table) {
  int command_pipe[2];
  int event_pipe[2];

  if (pipe(command_pipe) != 0) {
    return false;
  }

  if (pipe(event_pipe) != 0) {
    close(command_pipe[0]);
    close(command_pipe[1]);
    return false;
  }

  const pid_t pid = fork();

  if (pid < 0) {
    close(command_pipe[0]);
    close(command_pipe[1]);
    close(event_pipe[0]);
    close(event_pipe[1]);
    return false;
  }

  if (pid == 0) {
    close(command_pipe[1]);
    close(event_pipe[0]);
    const int status = driver_run(command_pipe[0], event_pipe[1], getpid());
    close(command_pipe[0]);
    close(event_pipe[1]);
    _exit(status);
  }

  close(command_pipe[0]);
  close(event_pipe[1]);

  const DriverEntry entry = {.pid = pid,
                             .command_fd = command_pipe[1],
                             .event_fd = event_pipe[0],
                             .state = DRIVER_STATE_AVAILABLE};

  if (!driver_table_add(table, entry)) {
    close(entry.command_fd);
    close(entry.event_fd);
    return false;
  }

  return true;
}

static bool parse_id(const char *text, pid_t *out) {
  char *end = nullptr;
  const long value = strtol(text, &end, 10);

  if (end == text || *end != '\0' || value <= 0 || value > INT32_MAX) {
    return false;
  }

  *out = (pid_t)value;
  return true;
}

static bool parse_seconds(const char *text, uint32_t *out) {
  char *end = nullptr;
  const long value = strtol(text, &end, 10);

  if (end == text || *end != '\0' || value <= 0 || value > INT32_MAX) {
    return false;
  }

  *out = (uint32_t)value;
  return true;
}

static void send_command(const DriverEntry *entry, const TaxiCommandType type,
                         const uint32_t seconds) {
  const TaxiCommandMessage command = {.type = type, .seconds = seconds};

  if (!taxi_protocol_write_command(entry->command_fd, &command)) {
    taxi_view_driver_exited(entry->pid);
  }
}

static void handle_create_driver(DriverTable *table, fd_set *master_set,
                                 int *max_fd) {
  if (!spawn_driver(table)) {
    taxi_view_driver_failed();
    return;
  }

  const DriverEntry *entry = driver_table_at(table, table->count - 1);
  FD_SET(entry->event_fd, master_set);

  if (entry->event_fd > *max_fd) {
    *max_fd = entry->event_fd;
  }

  taxi_view_driver_created(entry->pid);
}

static void handle_send_task(DriverTable *table, char *arguments) {
  char *pid_text = strtok(arguments, " \t\r\n");
  char *seconds_text = strtok(nullptr, " \t\r\n");
  pid_t pid;
  uint32_t seconds;

  if (pid_text == nullptr || seconds_text == nullptr ||
      !parse_id(pid_text, &pid) || !parse_seconds(seconds_text, &seconds)) {
    taxi_view_invalid_command();
    return;
  }

  DriverEntry *entry = driver_table_find(table, pid);

  if (entry == nullptr) {
    taxi_view_unknown_driver(pid);
    return;
  }

  send_command(entry, TAXI_COMMAND_TASK, seconds);
}

static void handle_get_status(DriverTable *table, char *arguments) {
  char *pid_text = strtok(arguments, " \t\r\n");
  pid_t pid;

  if (pid_text == nullptr || !parse_id(pid_text, &pid)) {
    taxi_view_invalid_command();
    return;
  }

  DriverEntry *entry = driver_table_find(table, pid);

  if (entry == nullptr) {
    taxi_view_unknown_driver(pid);
    return;
  }

  send_command(entry, TAXI_COMMAND_STATUS, 0);
}

static void handle_command(DriverTable *table, char *line, fd_set *master_set,
                           int *max_fd) {
  char *name = strtok(line, " \t\r\n");

  if (name == nullptr) {
    return;
  }

  if (strcmp(name, "create_driver") == 0) {
    handle_create_driver(table, master_set, max_fd);
  } else if (strcmp(name, "send_task") == 0) {
    handle_send_task(table, nullptr);
  } else if (strcmp(name, "get_status") == 0) {
    handle_get_status(table, nullptr);
  } else if (strcmp(name, "get_drivers") == 0) {
    taxi_view_drivers(table);
  } else {
    taxi_view_invalid_command();
  }
}

static bool handle_event(DriverEntry *entry) {
  TaxiEventMessage event;

  if (!taxi_protocol_read_event(entry->event_fd, &event)) {
    return false;
  }

  switch (event.type) {
  case TAXI_EVENT_TASK_ACCEPTED: {
    entry->state = DRIVER_STATE_BUSY;
    taxi_view_task_accepted(event.pid, event.seconds);
    break;
  }
  case TAXI_EVENT_STATUS_BUSY: {
    entry->state = DRIVER_STATE_BUSY;
    taxi_view_status_busy(event.pid, event.seconds);
    break;
  }
  case TAXI_EVENT_STATUS_AVAILABLE:
  case TAXI_EVENT_AVAILABLE: {
    entry->state = DRIVER_STATE_AVAILABLE;
    taxi_view_status_available(event.pid);
    break;
  }
  }

  return true;
}

static void close_driver(const DriverEntry *entry) {
  close(entry->command_fd);
  close(entry->event_fd);
  waitpid(entry->pid, nullptr, 0);
}

static void shutdown_drivers(DriverTable *table) {
  for (size_t index = 0; index < driver_table_count(table); index++) {
    const DriverEntry *entry = driver_table_at(table, index);
    const TaxiCommandMessage command = {.type = TAXI_COMMAND_SHUTDOWN,
                                        .seconds = 0};
    taxi_protocol_write_command(entry->command_fd, &command);
  }

  for (size_t index = 0; index < driver_table_count(table); index++) {
    close_driver(driver_table_at(table, index));
  }

  driver_table_free(table);
}

bool taxi_controller_run() {
  signal(SIGPIPE, SIG_IGN);

  DriverTable table;
  driver_table_initialize(&table);
  char line[LINE_CAPACITY];
  bool running = true;

  fd_set master_set;
  FD_ZERO(&master_set);
  FD_SET(STDIN_FILENO, &master_set);
  int max_fd = STDIN_FILENO;

  while (running) {
    fd_set read_set = master_set;

    if (select(max_fd + 1, &read_set, nullptr, nullptr, nullptr) < 0) {
      if (errno == EINTR) {
        continue;
      }

      running = false;
      continue;
    }

    for (size_t index = 0; index < driver_table_count(&table);) {
      DriverEntry *entry = driver_table_at(&table, index);

      if (FD_ISSET(entry->event_fd, &read_set)) {
        if (!handle_event(entry)) {
          const pid_t pid = entry->pid;
          FD_CLR(entry->event_fd, &master_set);
          close_driver(entry);
          driver_table_remove_at(&table, index);
          taxi_view_driver_exited(pid);
          continue;
        }
      }

      index++;
    }

    if (FD_ISSET(STDIN_FILENO, &read_set)) {
      if (fgets(line, sizeof(line), stdin) == nullptr) {
        running = false;
      } else {
        handle_command(&table, line, &master_set, &max_fd);
      }
    }
  }

  shutdown_drivers(&table);
  return true;
}
