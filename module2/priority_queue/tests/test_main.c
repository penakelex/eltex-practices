#include "unity.h"

void test_circular_buffer_run();
void test_priority_queue_run();

void setUp() {
}

void tearDown() {
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_circular_buffer_run);
  RUN_TEST(test_priority_queue_run);
  return UNITY_END();
}