#include <list.h>
#include <stdlib.h>
#include <string.h>

typedef struct ListNode {
  void *data;
  struct ListNode *previous;
  struct ListNode *next;
} ListNode;

struct List {
  ListNode *head;
  ListNode *tail;
  size_t count;
  size_t element_size;
  ListCompare compare;
  ListDestroy destroy;
};

List *list_create(const size_t element_size, const ListCompare compare,
                  const ListDestroy destroy) {
  if (element_size == 0 || compare == nullptr) {
    return nullptr;
  }

  List *list = calloc(1, sizeof *list);

  if (list == nullptr) {
    return nullptr;
  }

  list->element_size = element_size;
  list->compare = compare;
  list->destroy = destroy;
  return list;
}

void list_destroy(List *list) {
  if (list == nullptr) {
    return;
  }

  ListNode *node = list->head;

  while (node != nullptr) {
    ListNode *next = node->next;

    if (list->destroy != nullptr) {
      list->destroy(node->data);
    }

    free(node->data);
    free(node);
    node = next;
  }

  free(list);
}

static ListNode *list_find_position(List *list, const void *element) {
  ListNode *current = list->head;

  while (current != nullptr && list->compare(current->data, element) <= 0) {
    current = current->next;
  }

  return current;
}

bool list_add(List *list, const void *element) {
  if (list == nullptr || element == nullptr) {
    return false;
  }

  ListNode *node = malloc(sizeof *node);

  if (node == nullptr) {
    return false;
  }

  node->data = malloc(list->element_size);

  if (node->data == nullptr) {
    free(node);
    return false;
  }

  memcpy(node->data, element, list->element_size);
  node->previous = nullptr;
  node->next = nullptr;

  ListNode *current = list_find_position(list, element);

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

size_t list_count(const List *list) {
  if (list == nullptr) {
    return 0;
  }

  return list->count;
}

void *list_at(const List *list, const size_t index) {
  if (list == nullptr || index >= list->count) {
    return nullptr;
  }

  ListNode *node = list->head;

  for (size_t step = 0; step < index; ++step) {
    node = node->next;
  }

  return node->data;
}

bool list_remove_at(List *list, const size_t index) {
  if (list == nullptr || index >= list->count) {
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

  if (list->destroy != nullptr) {
    list->destroy(node->data);
  }

  free(node->data);
  free(node);
  --list->count;
  return true;
}

bool list_reorder_element(List *list, const size_t index) {
  if (list == nullptr || index >= list->count) {
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

  --list->count;

  ListNode *current = list_find_position(list, node->data);

  if (current == nullptr) {
    node->previous = list->tail;

    if (list->tail != nullptr) {
      list->tail->next = node;
    } else {
      list->head = node;
    }

    node->next = nullptr;
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

void list_for_each(const List *list, const ListVisitor visitor, void *context) {
  if (list == nullptr || visitor == nullptr) {
    return;
  }

  size_t index = 0;

  for (const ListNode *node = list->head; node != nullptr; node = node->next) {
    visitor(node->data, index, context);
    ++index;
  }
}