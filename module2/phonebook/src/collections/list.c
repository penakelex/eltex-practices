#include <collections/list.h>
#include <stdlib.h>

typedef struct ListNode {
  Contact contact;
  struct ListNode *previous;
  struct ListNode *next;
} ListNode;

typedef struct {
  ListNode *head;
  ListNode *tail;
  size_t count;
} ContactList;

static void *list_create(void) {
  return calloc(1, sizeof(ContactList));
}

static void list_destroy(void *self) {
  ContactList *list = self;

  if (list == nullptr) {
    return;
  }

  ListNode *node = list->head;

  while (node != nullptr) {
    ListNode *next = node->next;
    contact_free(&node->contact);
    free(node);
    node = next;
  }

  free(list);
}

static bool list_add(void *self, const Contact *contact) {
  ContactList *list = self;
  ListNode *node = malloc(sizeof *node);

  if (node == nullptr) {
    return false;
  }

  node->contact = *contact;
  node->previous = nullptr;
  node->next = nullptr;

  ListNode *current = list->head;

  while (current != nullptr &&
         contact_compare(&current->contact, contact) <= 0) {
    current = current->next;
  }

  if (current == nullptr) {
    node->previous = list->tail;

    if (list->tail != nullptr) {
      list->tail->next = node;
    } else {
      list->head = node;
    }

    list->tail = node;
  } else {
    node->next = current;
    node->previous = current->previous;

    if (current->previous != nullptr) {
      current->previous->next = node;
    } else {
      list->head = node;
    }

    current->previous = node;
  }

  ++list->count;
  return true;
}

static size_t list_count(const void *self) {
  return ((const ContactList *)self)->count;
}

static Contact *list_at(void *self, size_t index) {
  ContactList *list = self;

  if (index >= list->count) {
    return nullptr;
  }

  ListNode *node = list->head;

  for (size_t step = 0; step < index; ++step) {
    node = node->next;
  }

  return &node->contact;
}

static bool list_remove_at(void *self, size_t index) {
  ContactList *list = self;

  if (index >= list->count) {
    return false;
  }

  ListNode *node = list->head;

  for (size_t step = 0; step < index; ++step) {
    node = node->next;
  }

  if (node->previous != nullptr) {
    node->previous->next = node->next;
  } else {
    list->head = node->next;
  }

  if (node->next != nullptr) {
    node->next->previous = node->previous;
  } else {
    list->tail = node->previous;
  }

  contact_free(&node->contact);
  free(node);
  --list->count;
  return true;
}

static int list_node_compare(const void *left, const void *right) {
  const ListNode *left_node = *(const ListNode *const *)left;
  const ListNode *right_node = *(const ListNode *const *)right;
  return contact_compare(&left_node->contact, &right_node->contact);
}

static void list_reorder(void *self) {
  ContactList *list = self;

  if (list->count < 2) {
    return;
  }

  ListNode **nodes = malloc(list->count * sizeof *nodes);

  if (nodes == nullptr) {
    return;
  }

  size_t index = 0;

  for (ListNode *node = list->head; node != nullptr; node = node->next) {
    nodes[index++] = node;
  }

  qsort(nodes, list->count, sizeof *nodes, list_node_compare);

  for (index = 0; index < list->count; ++index) {
    nodes[index]->previous = (index > 0) ? nodes[index - 1] : nullptr;
    nodes[index]->next = (index + 1 < list->count) ? nodes[index + 1] : nullptr;
  }

  list->head = nodes[0];
  list->tail = nodes[list->count - 1];
  free(nodes);
}

static void list_dump(const void *self, ContactVisitor visitor, void *context) {
  const ContactList *list = self;
  size_t index = 0;

  for (const ListNode *node = list->head; node != nullptr; node = node->next) {
    visitor(&node->contact, index, context);
    ++index;
  }
}

static const CollectionOperations LIST_OPERATIONS = {
    .create = list_create,
    .destroy = list_destroy,
    .add = list_add,
    .count = list_count,
    .at = list_at,
    .remove_at = list_remove_at,
    .reorder = list_reorder,
    .dump = list_dump,
    .dump_tree = nullptr,
};

const CollectionOperations *list_operations(void) {
  return &LIST_OPERATIONS;
}