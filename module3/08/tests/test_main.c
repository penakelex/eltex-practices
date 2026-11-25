#include "unity.h"

void setUp() {
}

void tearDown() {
}

void test_packet_parser_run();
void test_packet_filter_run();
void test_packet_list_run();
void test_mac_resolver_run();
void test_packet_view_run();

int main() {
  UNITY_BEGIN();
  test_packet_parser_run();
  test_packet_filter_run();
  test_packet_list_run();
  test_mac_resolver_run();
  test_packet_view_run();
  return UNITY_END();
}
