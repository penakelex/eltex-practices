#include <collections/list.h>
#include <list.h>

typedef struct {
  ContactVisitor visitor;
  void *context;
} ListAdapterDump;

static int list_adapter_compare(const void *left, const void *right) {
  return contact_compare(left, right);
}

static void list_adapter_destroy_contact(void *element) {
  contact_free((Contact *)element);
}

static void *list_adapter_create(void) {
  return list_create(sizeof(Contact), list_adapter_compare,
                     list_adapter_destroy_contact);
}

static void list_adapter_destroy(void *self) {
  list_destroy(self);
}

static bool list_adapter_add(void *self, const Contact *contact) {
  return list_add(self, contact);
}

static size_t list_adapter_count(const void *self) {
  return list_count(self);
}

static Contact *list_adapter_at(void *self, const size_t index) {
  return list_at(self, index);
}

static bool list_adapter_remove_at(void *self, const size_t index) {
  return list_remove_at(self, index);
}

static bool list_adapter_reorder_element(void *self, const size_t index) {
  return list_reorder_element(self, index);
}

static void list_adapter_visit(const void *element, const size_t index,
                               void *context) {
  ListAdapterDump *dump = context;
  dump->visitor(element, index, dump->context);
}

static void list_adapter_dump(const void *self, const ContactVisitor visitor,
                              void *context) {
  ListAdapterDump dump = {.visitor = visitor, .context = context};
  list_for_each(self, list_adapter_visit, &dump);
}

static const CollectionOperations LIST_OPERATIONS = {
    .create = list_adapter_create,
    .destroy = list_adapter_destroy,
    .add = list_adapter_add,
    .count = list_adapter_count,
    .at = list_adapter_at,
    .remove_at = list_adapter_remove_at,
    .reorder_element = list_adapter_reorder_element,
    .dump = list_adapter_dump,
    .dump_tree = nullptr,
};

const CollectionOperations *list_operations(void) {
  return &LIST_OPERATIONS;
}