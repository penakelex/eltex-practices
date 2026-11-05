#include <collections/tree.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector.h>

enum : size_t { TREE_PREFIX_CAPACITY = 4096 };

typedef struct TreeNode {
  Contact contact;
  struct TreeNode *left;
  struct TreeNode *right;
  int height;
  size_t size;
} TreeNode;

typedef struct {
  TreeNode *root;
} Tree;

static int tree_node_height(const TreeNode *node) {
  if (node == nullptr) {
    return 0;
  }

  return node->height;
}

static size_t tree_node_size(const TreeNode *node) {
  if (node == nullptr) {
    return 0;
  }

  return node->size;
}

static void tree_update_node(TreeNode *node) {
  if (node == nullptr) {
    return;
  }

  const int left_height = tree_node_height(node->left);
  const int right_height = tree_node_height(node->right);
  const int maximum_height =
      left_height > right_height ? left_height : right_height;

  node->height = 1 + maximum_height;
  node->size = 1 + tree_node_size(node->left) + tree_node_size(node->right);
}

static TreeNode *tree_rotate_right(TreeNode *node) {
  TreeNode *left = node->left;
  TreeNode *middle = left->right;

  left->right = node;
  node->left = middle;

  tree_update_node(node);
  tree_update_node(left);
  return left;
}

static TreeNode *tree_rotate_left(TreeNode *node) {
  TreeNode *right = node->right;
  TreeNode *middle = right->left;

  right->left = node;
  node->right = middle;

  tree_update_node(node);
  tree_update_node(right);
  return right;
}

static TreeNode *tree_balance_node(TreeNode *node) {
  tree_update_node(node);

  const int balance =
      tree_node_height(node->left) - tree_node_height(node->right);

  if (balance > 1) {
    if (tree_node_height(node->left->left) <
        tree_node_height(node->left->right)) {
      node->left = tree_rotate_left(node->left);
    }

    return tree_rotate_right(node);
  }

  if (balance < -1) {
    if (tree_node_height(node->right->right) <
        tree_node_height(node->right->left)) {
      node->right = tree_rotate_right(node->right);
    }

    return tree_rotate_left(node);
  }

  return node;
}

static void tree_free_nodes(TreeNode *node) {
  if (node == nullptr) {
    return;
  }

  tree_free_nodes(node->left);
  tree_free_nodes(node->right);
  contact_free(&node->contact);
  free(node);
}

static void *tree_create(void) {
  return calloc(1, sizeof(Tree));
}

static void tree_destroy(void *self) {
  Tree *tree = self;

  if (tree == nullptr) {
    return;
  }

  tree_free_nodes(tree->root);
  free(tree);
}

static TreeNode *tree_insert_node(TreeNode *node, const Contact *contact,
                                  bool *success) {
  if (node == nullptr) {
    TreeNode *created = malloc(sizeof *created);

    if (created == nullptr) {
      *success = false;
      return nullptr;
    }

    created->contact = *contact;
    created->left = nullptr;
    created->right = nullptr;
    created->height = 1;
    created->size = 1;
    return created;
  }

  if (contact_compare(contact, &node->contact) < 0) {
    TreeNode *new_left = tree_insert_node(node->left, contact, success);

    if (!*success) {
      return node;
    }

    node->left = new_left;
  } else {
    TreeNode *new_right = tree_insert_node(node->right, contact, success);

    if (!*success) {
      return node;
    }

    node->right = new_right;
  }

  return tree_balance_node(node);
}

static bool tree_add(void *self, const Contact *contact) {
  if (self == nullptr || contact == nullptr) {
    return false;
  }

  Tree *tree = self;
  bool success = true;
  TreeNode *new_root = tree_insert_node(tree->root, contact, &success);

  if (!success) {
    return false;
  }

  tree->root = new_root;
  return true;
}

static size_t tree_count(const void *self) {
  if (self == nullptr) {
    return 0;
  }

  return tree_node_size(((const Tree *)self)->root);
}

static Contact *tree_at_impl(TreeNode *node, const size_t index) {
  if (node == nullptr) {
    return nullptr;
  }

  const size_t left_size = tree_node_size(node->left);

  if (index < left_size) {
    return tree_at_impl(node->left, index);
  }

  if (index == left_size) {
    return &node->contact;
  }

  return tree_at_impl(node->right, index - left_size - 1);
}

static Contact *tree_at(void *self, const size_t index) {
  if (self == nullptr) {
    return nullptr;
  }

  Tree *tree = self;

  if (index >= tree_node_size(tree->root)) {
    return nullptr;
  }

  return tree_at_impl(tree->root, index);
}

static TreeNode *tree_remove_minimum_node(TreeNode *node,
                                          Contact *out_contact) {
  if (node->left == nullptr) {
    *out_contact = node->contact;
    TreeNode *right = node->right;
    free(node);
    return right;
  }

  node->left = tree_remove_minimum_node(node->left, out_contact);
  return tree_balance_node(node);
}

static TreeNode *tree_remove_node(TreeNode *node, const size_t index,
                                  bool *success) {
  if (node == nullptr) {
    *success = false;
    return nullptr;
  }

  const size_t left_size = tree_node_size(node->left);

  if (index < left_size) {
    TreeNode *new_left = tree_remove_node(node->left, index, success);

    if (!*success) {
      return node;
    }

    node->left = new_left;
  } else if (index > left_size) {
    TreeNode *new_right =
        tree_remove_node(node->right, index - left_size - 1, success);

    if (!*success) {
      return node;
    }

    node->right = new_right;
  } else {
    if (node->left == nullptr || node->right == nullptr) {
      TreeNode *child = node->left != nullptr ? node->left : node->right;
      contact_free(&node->contact);
      free(node);
      *success = true;
      return child;
    }

    Contact moved_contact;
    node->right = tree_remove_minimum_node(node->right, &moved_contact);
    contact_free(&node->contact);
    node->contact = moved_contact;
    *success = true;
  }

  return tree_balance_node(node);
}

static bool tree_remove_at(void *self, const size_t index) {
  if (self == nullptr) {
    return false;
  }

  Tree *tree = self;

  if (index >= tree_node_size(tree->root)) {
    return false;
  }

  bool success = true;
  tree->root = tree_remove_node(tree->root, index, &success);
  return success;
}

static bool tree_reorder_element(void *self, const size_t index) {
  if (self == nullptr) {
    return false;
  }

  Tree *tree = self;

  if (index >= tree_node_size(tree->root)) {
    return false;
  }

  Contact *slot = tree_at_impl(tree->root, index);
  Contact moved = *slot;
  contact_initialize(slot);

  bool success = true;
  tree->root = tree_remove_node(tree->root, index, &success);

  if (!success) {
    contact_free(&moved);
    return false;
  }

  if (!tree_add(self, &moved)) {
    contact_free(&moved);
    return false;
  }

  contact_initialize(&moved);
  return true;
}

static void tree_dump_impl(const TreeNode *node, ContactVisitor visitor,
                           void *context, size_t *index) {
  if (node == nullptr) {
    return;
  }

  tree_dump_impl(node->left, visitor, context, index);
  visitor(&node->contact, *index, context);
  ++(*index);
  tree_dump_impl(node->right, visitor, context, index);
}

static void tree_dump(const void *self, ContactVisitor visitor, void *context) {
  if (self == nullptr) {
    return;
  }

  const Tree *tree = self;
  size_t index = 0;
  tree_dump_impl(tree->root, visitor, context, &index);
}

static void tree_dump_tree_impl(const TreeNode *node, const char *prefix,
                                const bool is_root, const bool is_last_child,
                                const char marker, TreeVisitor visitor,
                                void *context) {
  if (node == nullptr) {
    return;
  }

  if (is_root) {
    visitor(&node->contact, "", marker, context);
  } else {
    char line_prefix[TREE_PREFIX_CAPACITY];
    int written = snprintf(line_prefix, sizeof line_prefix, "%s%s", prefix,
                           is_last_child ? "└── " : "├── ");

    if (written < 0 || (size_t)written >= sizeof line_prefix) {
      return;
    }

    visitor(&node->contact, line_prefix, marker, context);
  }

  const bool has_left = node->left != nullptr;
  const bool has_right = node->right != nullptr;

  if (!has_left && !has_right) {
    return;
  }

  char child_prefix[TREE_PREFIX_CAPACITY];

  if (is_root) {
    child_prefix[0] = '\0';
  } else {
    int written = snprintf(child_prefix, sizeof child_prefix, "%s%s", prefix,
                           is_last_child ? "    " : "│   ");

    if (written < 0 || (size_t)written >= sizeof child_prefix) {
      return;
    }
  }

  if (has_left) {
    tree_dump_tree_impl(node->left, child_prefix, false, !has_right, 'L',
                        visitor, context);
  }

  if (has_right) {
    tree_dump_tree_impl(node->right, child_prefix, false, true, 'R', visitor,
                        context);
  }
}

static void tree_dump_tree(const void *self, TreeVisitor visitor,
                           void *context) {
  if (self == nullptr) {
    return;
  }

  const Tree *tree = self;
  tree_dump_tree_impl(tree->root, "", true, true, 'M', visitor, context);
}

static const CollectionOperations TREE_OPERATIONS = {
    .create = tree_create,
    .destroy = tree_destroy,
    .add = tree_add,
    .count = tree_count,
    .at = tree_at,
    .remove_at = tree_remove_at,
    .reorder_element = tree_reorder_element,
    .dump = tree_dump,
    .dump_tree = tree_dump_tree,
};

const CollectionOperations *tree_operations(void) {
  return &TREE_OPERATIONS;
}