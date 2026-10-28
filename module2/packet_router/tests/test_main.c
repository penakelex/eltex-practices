#include "unity.h"

void setUp() {
}

void tearDown() {
}

void test_packet_router_is_same_subnet_run();
void test_packet_router_run_invalid_run();
void test_packet_router_run_all_local_run();
void test_packet_stats_initialize_run();

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_packet_router_is_same_subnet_run);
  RUN_TEST(test_packet_router_run_invalid_run);
  RUN_TEST(test_packet_router_run_all_local_run);
  RUN_TEST(test_packet_stats_initialize_run);
  return UNITY_END();
}