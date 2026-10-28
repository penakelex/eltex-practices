#include "unity.h"

void test_ast_run();
void test_command_run();
void test_parser_run();
void test_calculator_run();

void setUp() {
}

void tearDown() {
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_ast_run);
  RUN_TEST(test_command_run);
  RUN_TEST(test_parser_run);
  RUN_TEST(test_calculator_run);
  return UNITY_END();
}