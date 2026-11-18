#pragma once

#include <stddef.h>
#include <stdint.h>

void view_usage(const char *program_name);

void view_error(const char *message);

void view_shared_memory_exists();

void view_shared_memory_unavailable();

void view_producer_started(size_t data_capacity);

void view_producer_appended(const uint32_t block_number, const uint32_t count);

void view_producer_waiting();

void view_producer_finished(const uint32_t block_count);

void view_consumer_started();

void view_consumer_processed(const uint32_t block_offset, const int32_t minimum,
                             const int32_t maximum, const uint32_t count);

void view_consumer_finished();
