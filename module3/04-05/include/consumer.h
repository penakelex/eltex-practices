#pragma once

#include "ipc.h"

bool consumer_run(const IpcOps *ipc_ops, const char *base_name,
                  const unsigned int poll_delay);
