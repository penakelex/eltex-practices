#include "unity.h"

void test_contact_run();
void test_phonebook_run();
void test_fields_run();

void setUp() {
}

void tearDown() {
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_contact_run);
  RUN_TEST(test_phonebook_run);
  RUN_TEST(test_fields_run);
  return UNITY_END();
}