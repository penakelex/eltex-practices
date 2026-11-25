#define _POSIX_C_SOURCE 202405L

#include "client_table.h"

#include <stdlib.h>

void client_table_initialize(ClientTable *table) {
  if (table == nullptr) {
    return;
  }

  table->head = nullptr;
}

void client_table_free(ClientTable *table) {
  if (table == nullptr) {
    return;
  }

  ClientEntry *entry = table->head;

  while (entry != nullptr) {
    ClientEntry *next = entry->next;
    free(entry);
    entry = next;
  }

  client_table_initialize(table);
}

static ClientEntry *find_entry(const ClientTable *table, uint32_t address,
                               uint16_t port) {
  for (ClientEntry *entry = table->head; entry != nullptr;
       entry = entry->next) {
    if (entry->address == address && entry->port == port) {
      return entry;
    }
  }

  return nullptr;
}

bool client_table_next_number(ClientTable *table, uint32_t address,
                              uint16_t port, uint32_t *out_number) {
  if (table == nullptr || out_number == nullptr) {
    return false;
  }

  ClientEntry *entry = find_entry(table, address, port);

  if (entry == nullptr) {
    entry = calloc(1, sizeof(*entry));

    if (entry == nullptr) {
      return false;
    }

    entry->address = address;
    entry->port = port;
    entry->next = table->head;
    table->head = entry;
  }

  entry->count++;
  *out_number = entry->count;
  return true;
}

void client_table_reset(ClientTable *table, uint32_t address, uint16_t port) {
  if (table == nullptr) {
    return;
  }

  ClientEntry **link = &table->head;

  while (*link != nullptr) {
    if ((*link)->address == address && (*link)->port == port) {
      ClientEntry *removed = *link;
      *link = removed->next;
      free(removed);
      return;
    }

    link = &(*link)->next;
  }
}
