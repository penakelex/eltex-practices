#include "taxi_view.h"

#include <stdio.h>

void taxi_view_driver_created(const pid_t pid) {
  printf("driver %d created\n", (int)pid);
  fflush(stdout);
}

void taxi_view_driver_exited(const pid_t pid) {
  printf("driver %d exited\n", (int)pid);
  fflush(stdout);
}

void taxi_view_driver_failed() {
  printf("failed to create driver\n");
  fflush(stdout);
}

void taxi_view_task_accepted(const pid_t pid, const uint32_t seconds) {
  printf("driver %d: task accepted, busy for %u s\n", (int)pid, seconds);
  fflush(stdout);
}

void taxi_view_status_available(const pid_t pid) {
  printf("driver %d: Available\n", (int)pid);
  fflush(stdout);
}

void taxi_view_status_busy(const pid_t pid, const uint32_t remaining_seconds) {
  printf("driver %d: Busy %u\n", (int)pid, remaining_seconds);
  fflush(stdout);
}

void taxi_view_drivers(const DriverTable *table) {
  const size_t count = driver_table_count(table);

  if (count == 0) {
    printf("no drivers\n");
    fflush(stdout);
    return;
  }

  for (size_t index = 0; index < count; index++) {
    const DriverEntry *entry = driver_table_at(table, index);

    if (entry->state == DRIVER_STATE_BUSY) {
      printf("driver %d: Busy\n", (int)entry->pid);
    } else {
      printf("driver %d: Available\n", (int)entry->pid);
    }
  }

  fflush(stdout);
}

void taxi_view_unknown_driver(const pid_t pid) {
  printf("driver %d not found\n", (int)pid);
  fflush(stdout);
}

void taxi_view_invalid_command() {
  printf("unknown command\n");
  fflush(stdout);
}
