#define _POSIX_C_SOURCE 202405L

#include "driver.h"

#include <errno.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "taxi_protocol.h"

static uint32_t remaining_seconds(const int timer_fd) {
  struct itimerspec spec;

  if (timerfd_gettime(timer_fd, &spec) != 0) {
    return 0;
  }

  uint32_t seconds = (uint32_t)spec.it_value.tv_sec;

  if (spec.it_value.tv_nsec > 0) {
    seconds++;
  }

  return seconds;
}

static void arm_timer(const int timer_fd, const uint32_t seconds) {
  struct itimerspec spec = {};
  spec.it_value.tv_sec = (time_t)seconds;
  timerfd_settime(timer_fd, 0, &spec, nullptr);
}

static bool send_event(const int event_fd, const TaxiEventType type,
                       const pid_t pid, const uint32_t seconds) {
  const TaxiEventMessage event = {.type = type, .pid = pid, .seconds = seconds};
  return taxi_protocol_write_event(event_fd, &event);
}

int driver_run(const int command_fd, const int event_fd, const pid_t pid) {
  const int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);

  if (timer_fd < 0) {
    return 1;
  }

  bool busy = false;
  int status = 0;

  fd_set master_set;
  FD_ZERO(&master_set);
  FD_SET(command_fd, &master_set);
  FD_SET(timer_fd, &master_set);
  const int max_fd = command_fd > timer_fd ? command_fd : timer_fd;

  while (true) {
    fd_set read_set = master_set;

    if (select(max_fd + 1, &read_set, nullptr, nullptr, nullptr) < 0) {
      if (errno == EINTR) {
        continue;
      }

      status = 1;
      break;
    }

    if (FD_ISSET(timer_fd, &read_set)) {
      uint64_t expirations;
      read(timer_fd, &expirations, sizeof(expirations));
      busy = false;
      send_event(event_fd, TAXI_EVENT_AVAILABLE, pid, 0);
    }

    if (!FD_ISSET(command_fd, &read_set)) {
      continue;
    }

    TaxiCommandMessage command;

    if (!taxi_protocol_read_command(command_fd, &command)) {
      break;
    }

    switch (command.type) {
    case TAXI_COMMAND_TASK: {
      if (busy) {
        send_event(event_fd, TAXI_EVENT_STATUS_BUSY, pid,
                   remaining_seconds(timer_fd));
        break;
      }

      busy = true;
      arm_timer(timer_fd, command.seconds);
      send_event(event_fd, TAXI_EVENT_TASK_ACCEPTED, pid, command.seconds);
      break;
    }
    case TAXI_COMMAND_STATUS: {
      if (busy) {
        send_event(event_fd, TAXI_EVENT_STATUS_BUSY, pid,
                   remaining_seconds(timer_fd));
      } else {
        send_event(event_fd, TAXI_EVENT_STATUS_AVAILABLE, pid, 0);
      }

      break;
    }
    case TAXI_COMMAND_SHUTDOWN: {
      close(timer_fd);
      return 0;
    }
    }
  }

  close(timer_fd);
  return status;
}
