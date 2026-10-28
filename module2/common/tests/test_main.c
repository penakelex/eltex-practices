#include "unity.h"

void test_vector_run();
void test_string_utils_run();
void test_input_utils_run();

void setUp() {
}

void tearDown() {
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_vector_run);
  RUN_TEST(test_string_utils_run);
  RUN_TEST(test_input_utils_run);
  return UNITY_END();
}