#include "unity.h"

void setUp() {
}

void tearDown() {
}

void test_protocol_run();
void test_message_run();

int main() {
  UNITY_BEGIN();
  test_protocol_run();
  test_message_run();
  return UNITY_END();
}
