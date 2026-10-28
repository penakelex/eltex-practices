#include <stdlib.h>
#include <string.h>
#include <vector.h>

constexpr size_t VECTOR_INITIAL_CAPACITY = 4;

void vector_initialize(Vector *vector, const size_t element_size,
                       const ElementDestructor element_destructor) {
  vector->data = nullptr;
  vector->element_size = element_size;
  vector->count = 0;
  vector->capacity = 0;
  vector->element_destructor = element_destructor;
}

void vector_clear(Vector *vector) {
  if (vector->element_destructor != nullptr) {
    for (size_t index = 0; index < vector->count; ++index) {
      vector->element_destructor((char *)vector->data +
                                 index * vector->element_size);
    }
  }

  vector->count = 0;
}

void vector_free(Vector *vector) {
  vector_clear(vector);
  free(vector->data);
  vector->data = nullptr;
  vector->count = 0;
  vector->capacity = 0;
}

bool vector_push_back(Vector *vector, const void *element) {
  if (vector->count == vector->capacity) {
    const size_t new_capacity = (vector->capacity == 0)
                                    ? VECTOR_INITIAL_CAPACITY
                                    : vector->capacity * 2;
    void *new_data = realloc(vector->data, new_capacity * vector->element_size);

    if (new_data == nullptr) {
      return false;
    }

    vector->data = new_data;
    vector->capacity = new_capacity;
  }

  memcpy((char *)vector->data + vector->count * vector->element_size, element,
         vector->element_size);
  ++vector->count;
  return true;
}

void *vector_at(const Vector *vector, const size_t index) {
  if (index >= vector->count) {
    return nullptr;
  }

  return (char *)vector->data + index * vector->element_size;
}

bool vector_remove_at(Vector *vector, const size_t index) {
  if (index >= vector->count) {
    return false;
  }

  if (vector->element_destructor != nullptr) {
    vector->element_destructor((char *)vector->data +
                               index * vector->element_size);
  }

  if (index + 1 < vector->count) {
    memmove((char *)vector->data + index * vector->element_size,
            (char *)vector->data + (index + 1) * vector->element_size,
            (vector->count - index - 1) * vector->element_size);
  }

  --vector->count;
  return true;
}