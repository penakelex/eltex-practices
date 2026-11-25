#include "unity.h"

void setUp() {
}

void tearDown() {
}

void test_client_registry_run();
void test_tcp_socket_run();
void test_file_transfer_run();

int main() {
  UNITY_BEGIN();
  test_client_registry_run();
  test_tcp_socket_run();
  test_file_transfer_run();
  return UNITY_END();
}
