#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "driver_table.h"

void taxi_view_driver_created(const pid_t pid);
void taxi_view_driver_exited(const pid_t pid);
void taxi_view_driver_failed();
void taxi_view_task_accepted(const pid_t pid, const uint32_t seconds);
void taxi_view_status_available(const pid_t pid);
void taxi_view_status_busy(const pid_t pid, const uint32_t remaining_seconds);
void taxi_view_drivers(const DriverTable *table);
void taxi_view_unknown_driver(const pid_t pid);
void taxi_view_invalid_command();
