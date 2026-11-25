#include "unity.h"

void setUp() {
}

void tearDown() {
}

void test_raw_packet_run();
void test_echo_protocol_run();
void test_client_table_run();

int main() {
  UNITY_BEGIN();
  test_raw_packet_run();
  test_echo_protocol_run();
  test_client_table_run();
  return UNITY_END();
}
