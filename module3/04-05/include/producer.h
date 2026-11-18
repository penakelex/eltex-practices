#pragma once

#include "ipc.h"

#include <stddef.h>

bool producer_run(const IpcOps *ipc_ops, const char *base_name,
                  const size_t data_capacity, const unsigned int produce_delay,
                  const unsigned int poll_delay);
