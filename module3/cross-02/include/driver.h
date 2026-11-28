#pragma once

#include <sys/types.h>

int driver_run(const int command_fd, const int event_fd, const pid_t pid);
