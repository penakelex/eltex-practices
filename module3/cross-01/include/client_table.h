#pragma once

#include <stdint.h>

typedef struct ClientEntry {
  uint32_t address;
  uint16_t port;
  uint32_t count;
  struct ClientEntry *next;
} ClientEntry;

typedef struct {
  ClientEntry *head;
} ClientTable;

void client_table_initialize(ClientTable *table);
void client_table_free(ClientTable *table);
bool client_table_next_number(ClientTable *table, uint32_t address,
                              uint16_t port, uint32_t *out_number);
void client_table_reset(ClientTable *table, uint32_t address, uint16_t port);
