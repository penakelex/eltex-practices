#pragma once

#include <command.h>
#include <stdbool.h>

typedef enum { AST_NUMBER, AST_UNARY, AST_BINARY } AstKind;

typedef struct AstNode {
  AstKind kind;

  union {
    double number;

    struct {
      UnaryFunction function;
      struct AstNode *operand;
    } unary;

    struct {
      BinaryFunction function;
      struct AstNode *left;
      struct AstNode *right;
    } binary;
  } data;
} AstNode;

AstNode *ast_number(double value);
AstNode *ast_unary(UnaryFunction function, AstNode *operand);
AstNode *ast_binary(BinaryFunction function, AstNode *left, AstNode *right);
void ast_free(AstNode *node);
bool ast_evaluate(const AstNode *node, double *result);