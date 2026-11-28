#define _POSIX_C_SOURCE 202405L

#include "driver_table.h"

#include <stdlib.h>
#include <string.h>

[[maybe_unused]] constexpr size_t INITIAL_CAPACITY = 8;

void driver_table_initialize(DriverTable *table) {
  if (table == nullptr) {
    return;
  }

  table->entries = nullptr;
  table->count = 0;
  table->capacity = 0;
}

void driver_table_free(DriverTable *table) {
  if (table == nullptr) {
    return;
  }

  free(table->entries);
  driver_table_initialize(table);
}

bool driver_table_add(DriverTable *table, const DriverEntry entry) {
  if (table == nullptr) {
    return false;
  }

  if (table->count == table->capacity) {
    const size_t capacity =
        table->capacity == 0 ? INITIAL_CAPACITY : table->capacity * 2;
    DriverEntry *entries = realloc(table->entries, capacity * sizeof(*entries));

    if (entries == nullptr) {
      return false;
    }

    table->entries = entries;
    table->capacity = capacity;
  }

  table->entries[table->count] = entry;
  table->count++;
  return true;
}

DriverEntry *driver_table_find(DriverTable *table, const pid_t pid) {
  if (table == nullptr) {
    return nullptr;
  }

  for (size_t index = 0; index < table->count; index++) {
    if (table->entries[index].pid == pid) {
      return &table->entries[index];
    }
  }

  return nullptr;
}

size_t driver_table_count(const DriverTable *table) {
  if (table == nullptr) {
    return 0;
  }

  return table->count;
}

DriverEntry *driver_table_at(const DriverTable *table, const size_t index) {
  if (table == nullptr || index >= table->count) {
    return nullptr;
  }

  return &table->entries[index];
}

void driver_table_remove_at(DriverTable *table, const size_t index) {
  if (table == nullptr || index >= table->count) {
    return;
  }

  table->count--;
  memmove(&table->entries[index], &table->entries[index + 1],
          (table->count - index) * sizeof(*table->entries));
}
