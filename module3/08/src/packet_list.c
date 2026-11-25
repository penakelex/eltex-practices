#define _POSIX_C_SOURCE 202405L

#include "packet_list.h"

#include <stdlib.h>
#include <string.h>

static constexpr size_t PACKET_LIST_INITIAL_CAPACITY = 16;

void packet_list_initialize(PacketList *list) {
  if (list == nullptr) {
    return;
  }

  list->items = nullptr;
  list->count = 0;
  list->capacity = 0;
}

void packet_list_free(PacketList *list) {
  if (list == nullptr) {
    return;
  }

  free(list->items);
  packet_list_initialize(list);
}

bool packet_list_add(PacketList *list, const PacketInfo *packet) {
  if (list == nullptr || packet == nullptr) {
    return false;
  }

  if (list->count == list->capacity) {
    const size_t capacity =
        list->capacity == 0 ? PACKET_LIST_INITIAL_CAPACITY : list->capacity * 2;
    PacketInfo *items = realloc(list->items, capacity * sizeof(*items));

    if (items == nullptr) {
      return false;
    }

    list->items = items;
    list->capacity = capacity;
  }

  list->items[list->count++] = *packet;
  return true;
}
