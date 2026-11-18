#pragma once

#include <stddef.h>

bool producer_run(const char *key_path, const size_t data_capacity,
                  const unsigned int produce_delay,
                  const unsigned int poll_delay);
