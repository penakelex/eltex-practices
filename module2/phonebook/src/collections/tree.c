#include <collections/tree.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector.h>

enum : size_t { TREE_REBALANCE_THRESHOLD = 4 };

enum : size_t { TREE_PREFIX_CAPACITY = 4096 };

typedef struct TreeNode {
  Contact contact;
  struct TreeNode *left;
  struct TreeNode *right;
} TreeNode;

typedef struct {
  TreeNode *root;
  size_t count;
  size_t operations_since_rebalance;
} Tree;

static void free_contact_element(void *element) {
  contact_free((Contact *)element);
}

static void tree_collect_and_free(TreeNode *node, Vector *vector) {
  if (node == nullptr) {
    return;
  }

  tree_collect_and_free(node->left, vector);

  if (!vector_push_back(vector, &node->contact)) {
    contact_free(&node->contact);
  }

  TreeNode *right = node->right;
  free(node);
  tree_collect_and_free(right, vector);
}

static TreeNode *tree_build_balanced(Vector *vector, const size_t left,
                                     const size_t right) {
  if (left >= right) {
    return nullptr;
  }

  const size_t middle = (left + right) / 2;
  TreeNode *node = malloc(sizeof *node);

  if (node == nullptr) {
    return nullptr;
  }

  node->contact = *(Contact *)vector_at(vector, middle);
  node->left = tree_build_balanced(vector, left, middle);
  node->right = tree_build_balanced(vector, middle + 1, right);
  return node;
}

static void tree_reorder(void *self) {
  Tree *tree = self;
  Vector vector;
  vector_initialize(&vector, sizeof(Contact), free_contact_element);
  tree_collect_and_free(tree->root, &vector);

  tree->root = tree_build_balanced(&vector, 0, vector.count);
  tree->count = vector.count;
  tree->operations_since_rebalance = 0;

  vector.element_destructor = nullptr;
  vector_free(&vector);
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
    return created;
  }

  if (contact_compare(&node->contact, contact) <= 0) {
    node->right = tree_insert_node(node->right, contact, success);
  } else {
    node->left = tree_insert_node(node->left, contact, success);
  }

  return node;
}

static bool tree_add(void *self, const Contact *contact) {
  Tree *tree = self;
  bool success = true;
  TreeNode *new_root = tree_insert_node(tree->root, contact, &success);

  if (!success) {
    return false;
  }

  tree->root = new_root;
  ++tree->count;
  ++tree->operations_since_rebalance;

  if (tree->operations_since_rebalance >= TREE_REBALANCE_THRESHOLD) {
    tree_reorder(tree);
  }

  return true;
}

static size_t tree_count(const void *self) {
  return ((const Tree *)self)->count;
}

static Contact *tree_at_impl(TreeNode *node, const size_t index,
                             size_t *visited) {
  if (node == nullptr) {
    return nullptr;
  }

  Contact *left_result = tree_at_impl(node->left, index, visited);

  if (left_result != nullptr) {
    return left_result;
  }

  if (*visited == index) {
    return &node->contact;
  }

  ++(*visited);
  return tree_at_impl(node->right, index, visited);
}

static Contact *tree_at(void *self, const size_t index) {
  Tree *tree = self;

  if (index >= tree->count) {
    return nullptr;
  }

  size_t visited = 0;
  return tree_at_impl(tree->root, index, &visited);
}

static bool tree_remove_at(void *self, const size_t index) {
  Tree *tree = self;

  if (index >= tree->count) {
    return false;
  }

  Vector vector;
  vector_initialize(&vector, sizeof(Contact), free_contact_element);
  tree_collect_and_free(tree->root, &vector);
  tree->root = nullptr;
  tree->count = 0;

  vector_remove_at(&vector, index);
  tree->root = tree_build_balanced(&vector, 0, vector.count);
  tree->count = vector.count;
  tree->operations_since_rebalance = 0;

  vector.element_destructor = nullptr;
  vector_free(&vector);
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
  const Tree *tree = self;
  size_t index = 0;
  tree_dump_impl(tree->root, visitor, context, &index);
}

static void tree_dump_tree_impl(const TreeNode *node, const char *prefix,
                                const bool is_root, const bool is_last_child,
                                TreeVisitor visitor, void *context) {
  if (node == nullptr) {
    return;
  }

  if (is_root) {
    visitor(&node->contact, "", context);
  } else {
    char line_prefix[TREE_PREFIX_CAPACITY];
    int written = snprintf(line_prefix, sizeof line_prefix, "%s%s", prefix,
                           is_last_child ? "└── " : "├── ");

    if (written < 0 || (size_t)written >= sizeof line_prefix) {
      return;
    }

    visitor(&node->contact, line_prefix, context);
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
    tree_dump_tree_impl(node->left, child_prefix, false, !has_right, visitor,
                        context);
  }

  if (has_right) {
    tree_dump_tree_impl(node->right, child_prefix, false, true, visitor,
                        context);
  }
}

static void tree_dump_tree(const void *self, TreeVisitor visitor,
                           void *context) {
  const Tree *tree = self;
  tree_dump_tree_impl(tree->root, "", true, true, visitor, context);
}

static const CollectionOperations TREE_OPERATIONS = {
    .create = tree_create,
    .destroy = tree_destroy,
    .add = tree_add,
    .count = tree_count,
    .at = tree_at,
    .remove_at = tree_remove_at,
    .reorder = tree_reorder,
    .dump = tree_dump,
    .dump_tree = tree_dump_tree,
};

const CollectionOperations *tree_operations(void) {
  return &TREE_OPERATIONS;
}