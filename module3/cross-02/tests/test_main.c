#include "unity.h"

void setUp() {
}

void tearDown() {
}

void test_taxi_protocol_run();
void test_driver_table_run();

int main() {
  UNITY_BEGIN();
  test_taxi_protocol_run();
  test_driver_table_run();
  return UNITY_END();
}
