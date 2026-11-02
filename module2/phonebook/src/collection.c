#include <collection.h>
#include <collections/array.h>
#include <collections/list.h>
#include <collections/tree.h>
#include <string.h>

const CollectionOperations *collection_get_operations(const char *name) {
  if (name == nullptr) {
    return nullptr;
  }

  if (strcmp(name, "array") == 0) {
    return array_operations();
  }

  if (strcmp(name, "list") == 0) {
    return list_operations();
  }

  if (strcmp(name, "tree") == 0) {
    return tree_operations();
  }

  return nullptr;
}