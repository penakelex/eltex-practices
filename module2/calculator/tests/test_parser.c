#include "unity.h"
#include <ast.h>
#include <math.h>
#include <parser.h>

constexpr double TEST_PI = 3.14159265358979323846;
constexpr double TEST_E = 2.71828182845904523536;

static bool parse_and_evaluate(const char *expression, double expected,
                               double epsilon) {
  AstNode *root = NULL;
  const char *error = NULL;
  const ParserStatus status = parser_parse(expression, &root, &error);

  if (status != PARSER_OK || root == NULL) {
    return false;
  }

  double value = 0.0;
  const bool ok = ast_evaluate(root, &value);
  ast_free(root);

  return ok && fabs(value - expected) <= epsilon;
}

static void test_simple_number() {
  TEST_ASSERT_TRUE(parse_and_evaluate("42", 42.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("3.14", 3.14, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("0", 0.0, 1e-9));
}

static void test_constants() {
  TEST_ASSERT_TRUE(parse_and_evaluate("pi", TEST_PI, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("e", TEST_E, 1e-9));
}

static void test_binary_precedence() {
  TEST_ASSERT_TRUE(parse_and_evaluate("2 + 3", 5.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("2 - 3", -1.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("2 * 3", 6.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("6 / 3", 2.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("2 + 3 * 4", 14.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("2 * 3 + 4", 10.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("(2 + 3) * 4", 20.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("2 * (3 + 4)", 14.0, 1e-9));
}

static void test_power_right_associativity() {
  TEST_ASSERT_TRUE(parse_and_evaluate("2 ^ 3", 8.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("2 ^ 3 ^ 2", 512.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("(2 ^ 3) ^ 2", 64.0, 1e-9));
}

static void test_unary_function() {
  TEST_ASSERT_TRUE(parse_and_evaluate("sqrt(16)", 4.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("sin(0)", 0.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("abs(neg(5))", 5.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("abs(-5)", 5.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("sqrt(4) + sqrt(9)", 5.0, 1e-9));
}

static void test_prefix_minus() {
  TEST_ASSERT_TRUE(parse_and_evaluate("-5", -5.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("2 + -3", -1.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("2 - -3", 5.0, 1e-9));
  TEST_ASSERT_TRUE(parse_and_evaluate("+5", 5.0, 1e-9));
}

static void test_parse_error() {
  AstNode *root = NULL;
  const char *error = NULL;

  TEST_ASSERT_EQUAL_INT(PARSER_ERROR_SYNTAX,
                        parser_parse("2 +", &root, &error));
  TEST_ASSERT_NULL(root);

  TEST_ASSERT_EQUAL_INT(PARSER_ERROR_SYNTAX, parser_parse("", &root, &error));
  TEST_ASSERT_NULL(root);

  TEST_ASSERT_EQUAL_INT(PARSER_ERROR_SYNTAX,
                        parser_parse("foo", &root, &error));
  TEST_ASSERT_NULL(root);

  TEST_ASSERT_EQUAL_INT(PARSER_ERROR_SYNTAX,
                        parser_parse("(2 + 3", &root, &error));
  TEST_ASSERT_NULL(root);

  TEST_ASSERT_EQUAL_INT(PARSER_ERROR_SYNTAX,
                        parser_parse("2 3", &root, &error));
  TEST_ASSERT_NULL(root);
}

void test_parser_run() {
  test_simple_number();
  test_constants();
  test_binary_precedence();
  test_power_right_associativity();
  test_unary_function();
  test_prefix_minus();
  test_parse_error();
}