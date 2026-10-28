#include "unity.h"
#include <ast.h>

static double add(double left, double right) {
  return left + right;
}

static double negate(double value) {
  return -value;
}

static void test_number_node() {
  AstNode *node = ast_number(3.14);

  TEST_ASSERT_NOT_NULL(node);
  TEST_ASSERT_EQUAL_INT(AST_NUMBER, node->kind);

  double value = 0.0;
  TEST_ASSERT_TRUE(ast_evaluate(node, &value));
  TEST_ASSERT_EQUAL_DOUBLE(3.14, value);

  ast_free(node);
}

static void test_binary_node() {
  AstNode *node = ast_binary(add, ast_number(2.0), ast_number(3.0));

  TEST_ASSERT_NOT_NULL(node);
  TEST_ASSERT_EQUAL_INT(AST_BINARY, node->kind);

  double value = 0.0;
  TEST_ASSERT_TRUE(ast_evaluate(node, &value));
  TEST_ASSERT_EQUAL_DOUBLE(5.0, value);

  ast_free(node);
}

static void test_unary_node() {
  AstNode *node = ast_unary(negate, ast_number(7.0));

  TEST_ASSERT_NOT_NULL(node);
  TEST_ASSERT_EQUAL_INT(AST_UNARY, node->kind);

  double value = 0.0;
  TEST_ASSERT_TRUE(ast_evaluate(node, &value));
  TEST_ASSERT_EQUAL_DOUBLE(-7.0, value);

  ast_free(node);
}

static void test_nested_evaluation() {
  AstNode *node =
      ast_binary(add, ast_number(2.0), ast_unary(negate, ast_number(3.0)));

  TEST_ASSERT_NOT_NULL(node);

  double value = 0.0;
  TEST_ASSERT_TRUE(ast_evaluate(node, &value));
  TEST_ASSERT_EQUAL_DOUBLE(-1.0, value);

  ast_free(node);
}

static void test_free_null_is_safe() {
  ast_free(NULL);
  TEST_PASS();
}

void test_ast_run() {
  test_number_node();
  test_binary_node();
  test_unary_node();
  test_nested_evaluation();
  test_free_null_is_safe();
}