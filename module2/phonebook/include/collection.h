#pragma once

#include <contact.h>
#include <stddef.h>

typedef void (*ContactVisitor)(const Contact *contact, size_t index,
                               void *context);

typedef void (*TreeVisitor)(const Contact *contact, const char *prefix,
                            char marker, void *context);

typedef struct {
  void *(*create)(void);
  void (*destroy)(void *self);
  bool (*add)(void *self, const Contact *contact);
  size_t (*count)(const void *self);
  Contact *(*at)(void *self, size_t index);
  bool (*remove_at)(void *self, size_t index);
  bool (*reorder_element)(void *self, size_t index);
  void (*dump)(const void *self, ContactVisitor visitor, void *context);
  void (*dump_tree)(const void *self, TreeVisitor visitor, void *context);
} CollectionOperations;

const CollectionOperations *collection_get_operations(const char *name);