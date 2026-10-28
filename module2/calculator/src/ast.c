#include <ast.h>
#include <stddef.h>
#include <stdlib.h>

AstNode *ast_number(const double value) {
  AstNode *node = malloc(sizeof(AstNode));

  if (node == nullptr) {
    return nullptr;
  }

  node->kind = AST_NUMBER;
  node->data.number = value;
  return node;
}

AstNode *ast_unary(const UnaryFunction function, AstNode *operand) {
  AstNode *node = malloc(sizeof(AstNode));

  if (node == nullptr) {
    return nullptr;
  }

  node->kind = AST_UNARY;
  node->data.unary.function = function;
  node->data.unary.operand = operand;
  return node;
}

AstNode *ast_binary(const BinaryFunction function, AstNode *left,
                    AstNode *right) {
  AstNode *node = malloc(sizeof(AstNode));

  if (node == nullptr) {
    return nullptr;
  }

  node->kind = AST_BINARY;
  node->data.binary.function = function;
  node->data.binary.left = left;
  node->data.binary.right = right;
  return node;
}

void ast_free(AstNode *node) {
  if (node == nullptr) {
    return;
  }

  if (node->kind == AST_UNARY) {
    ast_free(node->data.unary.operand);
  } else if (node->kind == AST_BINARY) {
    ast_free(node->data.binary.left);
    ast_free(node->data.binary.right);
  }

  free(node);
}

bool ast_evaluate(const AstNode *node, double *result) {
  if (node == nullptr || result == nullptr) {
    return false;
  }

  switch (node->kind) {
  case AST_NUMBER: {
    *result = node->data.number;
    return true;
  }
  case AST_UNARY: {
    double value = 0.0;

    if (!ast_evaluate(node->data.unary.operand, &value)) {
      return false;
    }

    *result = node->data.unary.function(value);
    return true;
  }
  case AST_BINARY: {
    double left = 0.0;
    double right = 0.0;

    if (!ast_evaluate(node->data.binary.left, &left)) {
      return false;
    }

    if (!ast_evaluate(node->data.binary.right, &right)) {
      return false;
    }

    *result = node->data.binary.function(left, right);
    return true;
  }
  }

  return false;
}