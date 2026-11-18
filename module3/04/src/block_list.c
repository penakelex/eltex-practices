#define _POSIX_C_SOURCE 202405L

#include "block_list.h"

#include <string.h>

size_t block_list_header_size() {
  return sizeof(BlockListHeader);
}

static size_t block_size(const BlockHeader *header) {
  return sizeof(BlockHeader) + (size_t)header->count * sizeof(int32_t);
}

static bool offset_in_bounds(const uint32_t offset, const size_t capacity) {
  return (size_t)offset + sizeof(BlockHeader) <= capacity;
}

static const BlockHeader *
walk_to_last(const void *region, const size_t capacity, uint32_t *offset) {
  const BlockListHeader *list = region;
  auto current = list->head_offset;

  if (current == BLOCK_LIST_END) {
    return nullptr;
  }

  const BlockHeader *header = nullptr;

  while (current != BLOCK_LIST_END) {
    if (!offset_in_bounds(current, capacity)) {
      return nullptr;
    }

    header = (const BlockHeader *)((const char *)region + current);

    if (header->next_offset == BLOCK_LIST_END) {
      *offset = current;
      return header;
    }

    current = header->next_offset;
  }

  return nullptr;
}

bool block_list_initialize(void *region, const size_t capacity) {
  if (region == nullptr || capacity < sizeof(BlockListHeader)) {
    return false;
  }

  BlockListHeader *list = region;
  list->head_offset = BLOCK_LIST_END;
  memset((char *)region + sizeof(BlockListHeader), 0,
         capacity - sizeof(BlockListHeader));
  return true;
}

bool block_list_append(void *region, const size_t capacity,
                       const int32_t *values, const uint32_t count,
                       uint32_t *offset_out) {
  if (region == nullptr || values == nullptr || count == 0 ||
      offset_out == nullptr) {
    return false;
  }

  const size_t needed = sizeof(BlockHeader) + (size_t)count * sizeof(int32_t);

  BlockListHeader *list = region;
  uint32_t last_offset = 0;
  const BlockHeader *last = walk_to_last(region, capacity, &last_offset);
  uint32_t new_offset = 0;

  if (last == nullptr) {
    new_offset = (uint32_t)sizeof(BlockListHeader);
  } else {
    new_offset = last_offset + (uint32_t)block_size(last);
  }

  if ((size_t)new_offset + needed > capacity) {
    return false;
  }

  BlockHeader *header = (BlockHeader *)((char *)region + new_offset);
  header->count = count;
  header->next_offset = BLOCK_LIST_END;
  memcpy((char *)header + sizeof(BlockHeader), values,
         (size_t)count * sizeof(int32_t));

  if (last == nullptr) {
    list->head_offset = new_offset;
  } else {
    ((BlockHeader *)((char *)region + last_offset))->next_offset = new_offset;
  }

  *offset_out = new_offset;
  return true;
}

bool block_list_find_unprocessed(const void *region, const size_t capacity,
                                 uint32_t *offset_out) {
  if (region == nullptr || offset_out == nullptr) {
    return false;
  }

  const BlockListHeader *list = region;
  auto current = list->head_offset;

  while (current != BLOCK_LIST_END) {
    if (!offset_in_bounds(current, capacity)) {
      return false;
    }

    const BlockHeader *header =
        (const BlockHeader *)((const char *)region + current);

    if (header->count > 0) {
      *offset_out = current;
      return true;
    }

    current = header->next_offset;
  }

  return false;
}

BlockHeader *block_list_header_at(void *region, const uint32_t offset) {
  if (region == nullptr) {
    return nullptr;
  }

  return (BlockHeader *)((char *)region + offset);
}

const BlockHeader *block_list_header_at_const(const void *region,
                                              const uint32_t offset) {
  if (region == nullptr) {
    return nullptr;
  }

  return (const BlockHeader *)((const char *)region + offset);
}

int32_t *block_list_values_at(void *region, const uint32_t offset) {
  if (region == nullptr) {
    return nullptr;
  }

  return (int32_t *)((char *)region + offset + sizeof(BlockHeader));
}

const int32_t *block_list_values_at_const(const void *region,
                                          const uint32_t offset) {
  if (region == nullptr) {
    return nullptr;
  }

  return (const int32_t *)((const char *)region + offset + sizeof(BlockHeader));
}

void block_list_mark_processed(void *region, const uint32_t offset) {
  if (region == nullptr) {
    return;
  }

  BlockHeader *header = block_list_header_at(region, offset);
  header->count = 0;
}

bool block_list_all_processed(const void *region, const size_t capacity) {
  if (region == nullptr) {
    return false;
  }

  const BlockListHeader *list = region;
  auto current = list->head_offset;

  while (current != BLOCK_LIST_END) {
    if (!offset_in_bounds(current, capacity)) {
      return false;
    }

    const BlockHeader *header =
        (const BlockHeader *)((const char *)region + current);

    if (header->count > 0) {
      return false;
    }

    current = header->next_offset;
  }

  return true;
}

bool block_list_is_empty(const void *region) {
  if (region == nullptr) {
    return true;
  }

  const BlockListHeader *list = region;
  return list->head_offset == BLOCK_LIST_END;
}
