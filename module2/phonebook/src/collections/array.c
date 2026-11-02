#include <collections/array.h>
#include <stdlib.h>
#include <vector.h>

static void free_contact_element(void *element) {
  contact_free((Contact *)element);
}

static void *array_create(void) {
  Vector *vector = malloc(sizeof *vector);

  if (vector == nullptr) {
    return nullptr;
  }

  vector_initialize(vector, sizeof(Contact), free_contact_element);
  return vector;
}

static void array_destroy(void *self) {
  Vector *vector = self;

  if (vector == nullptr) {
    return;
  }

  vector_free(vector);
  free(vector);
}

static bool array_add(void *self, const Contact *contact) {
  Vector *vector = self;
  size_t position = vector->count;

  for (size_t index = 0; index < vector->count; ++index) {
    if (contact_compare(vector_at(vector, index), contact) > 0) {
      position = index;
      break;
    }
  }

  return vector_insert_at(vector, position, contact);
}

static size_t array_count(const void *self) {
  return ((const Vector *)self)->count;
}

static Contact *array_at(void *self, size_t index) {
  return vector_at((Vector *)self, index);
}

static bool array_remove_at(void *self, size_t index) {
  return vector_remove_at((Vector *)self, index);
}

static int array_qsort_compare(const void *left, const void *right) {
  return contact_compare(left, right);
}

static void array_reorder(void *self) {
  Vector *vector = self;

  if (vector->count < 2) {
    return;
  }

  qsort(vector->data, vector->count, vector->element_size, array_qsort_compare);
}

static void array_dump(const void *self, ContactVisitor visitor,
                       void *context) {
  const Vector *vector = self;

  for (size_t index = 0; index < vector->count; ++index) {
    const Contact *contact = vector_at(vector, index);
    visitor(contact, index, context);
  }
}

static const CollectionOperations ARRAY_OPERATIONS = {
    .create = array_create,
    .destroy = array_destroy,
    .add = array_add,
    .count = array_count,
    .at = array_at,
    .remove_at = array_remove_at,
    .reorder = array_reorder,
    .dump = array_dump,
    .dump_tree = nullptr,
};

const CollectionOperations *array_operations(void) {
  return &ARRAY_OPERATIONS;
}