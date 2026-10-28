#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef void (*ElementDestructor)(void *element);

typedef struct {
  void *data;
  size_t element_size;
  size_t count;
  size_t capacity;
  ElementDestructor element_destructor;
} Vector;

void vector_initialize(Vector *vector, size_t element_size,
                       ElementDestructor element_destructor);
void vector_free(Vector *vector);
void vector_clear(Vector *vector);
bool vector_push_back(Vector *vector, const void *element);
void *vector_at(const Vector *vector, size_t index);
bool vector_remove_at(Vector *vector, size_t index);