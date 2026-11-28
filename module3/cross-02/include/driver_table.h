#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef enum : int32_t {
  DRIVER_STATE_AVAILABLE,
  DRIVER_STATE_BUSY
} DriverState;

typedef struct {
  pid_t pid;
  int command_fd;
  int event_fd;
  DriverState state;
} DriverEntry;

typedef struct {
  DriverEntry *entries;
  size_t count;
  size_t capacity;
} DriverTable;

void driver_table_initialize(DriverTable *table);
void driver_table_free(DriverTable *table);
bool driver_table_add(DriverTable *table, const DriverEntry entry);
DriverEntry *driver_table_find(DriverTable *table, const pid_t pid);
size_t driver_table_count(const DriverTable *table);
DriverEntry *driver_table_at(const DriverTable *table, const size_t index);
void driver_table_remove_at(DriverTable *table, const size_t index);
