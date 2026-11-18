#include "unity.h"

void setUp() {
}

void tearDown() {
}

void test_block_list_run();

void test_shared_memory_run();

int main() {
  UNITY_BEGIN();
  test_block_list_run();
  test_shared_memory_run();
  return UNITY_END();
}
