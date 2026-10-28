#include "unity.h"
#include <calculator.h>

static void test_valid_expressions() {
  double result = 0.0;
  const char *error = NULL;

  TEST_ASSERT_EQUAL_INT(CALCULATOR_OK,
                        calculator_evaluate("2 + 2", &result, &error));
  TEST_ASSERT_EQUAL_DOUBLE(4.0, result);

  TEST_ASSERT_EQUAL_INT(CALCULATOR_OK,
                        calculator_evaluate("2 + 3 * 4", &result, &error));
  TEST_ASSERT_EQUAL_DOUBLE(14.0, result);

  TEST_ASSERT_EQUAL_INT(CALCULATOR_OK,
                        calculator_evaluate("sqrt(16) + 1", &result, &error));
  TEST_ASSERT_EQUAL_DOUBLE(5.0, result);

  TEST_ASSERT_EQUAL_INT(CALCULATOR_OK,
                        calculator_evaluate("2 ^ 3 ^ 2", &result, &error));
  TEST_ASSERT_EQUAL_DOUBLE(512.0, result);
}

static void test_division_by_zero() {
  double result = 0.0;
  const char *error = NULL;

  TEST_ASSERT_EQUAL_INT(CALCULATOR_ERROR_DOMAIN,
                        calculator_evaluate("1 / 0", &result, &error));
}

static void test_parse_error() {
  double result = 0.0;
  const char *error = NULL;

  TEST_ASSERT_EQUAL_INT(CALCULATOR_ERROR_PARSE,
                        calculator_evaluate("", &result, &error));

  TEST_ASSERT_EQUAL_INT(CALCULATOR_ERROR_PARSE,
                        calculator_evaluate("abc", &result, &error));

  TEST_ASSERT_EQUAL_INT(CALCULATOR_ERROR_PARSE,
                        calculator_evaluate("2 +", &result, &error));
  TEST_ASSERT_NOT_NULL(error);
}

static void test_null_arguments() {
  double result = 0.0;

  TEST_ASSERT_EQUAL_INT(CALCULATOR_ERROR_PARSE,
                        calculator_evaluate(NULL, &result, NULL));

  TEST_ASSERT_EQUAL_INT(CALCULATOR_ERROR_PARSE,
                        calculator_evaluate("2", NULL, NULL));
}

void test_calculator_run() {
  test_valid_expressions();
  test_division_by_zero();
  test_parse_error();
  test_null_arguments();
}