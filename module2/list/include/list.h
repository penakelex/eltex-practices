#pragma once

#include <stddef.h>

typedef int (*ListCompare)(const void *left, const void *right);
typedef void (*ListDestroy)(void *element);
typedef void (*ListVisitor)(const void *element, size_t index, void *context);

typedef struct List List;

List *list_create(size_t element_size, ListCompare compare,
                  ListDestroy destroy);
void list_destroy(List *list);
bool list_add(List *list, const void *element);
size_t list_count(const List *list);
void *list_at(const List *list, size_t index);
bool list_remove_at(List *list, size_t index);
bool list_reorder_element(List *list, size_t index);
void list_for_each(const List *list, ListVisitor visitor, void *context);