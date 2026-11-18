#pragma once

#include <stddef.h>
#include <stdint.h>

[[maybe_unused]] constexpr uint32_t BLOCK_LIST_END = UINT32_MAX;

typedef struct {
  uint32_t count;
  uint32_t next_offset;
} BlockHeader;

typedef struct {
  uint32_t head_offset;
} BlockListHeader;

size_t block_list_header_size();

bool block_list_initialize(void *region, const size_t capacity);

bool block_list_append(void *region, const size_t capacity,
                       const int32_t *values, const uint32_t count,
                       uint32_t *offset_out);

bool block_list_find_unprocessed(const void *region, const size_t capacity,
                                 uint32_t *offset_out);

BlockHeader *block_list_header_at(void *region, const uint32_t offset);

const BlockHeader *block_list_header_at_const(const void *region,
                                              const uint32_t offset);

int32_t *block_list_values_at(void *region, const uint32_t offset);

const int32_t *block_list_values_at_const(const void *region,
                                          const uint32_t offset);

void block_list_mark_processed(void *region, const uint32_t offset);

bool block_list_all_processed(const void *region, const size_t capacity);

bool block_list_is_empty(const void *region);
