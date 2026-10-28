#include "unity.h"
#include <contact.h>
#include <stdlib.h>
#include <string.h>

static void test_initialize_is_empty() {
  Contact contact;
  contact_initialize(&contact);

  TEST_ASSERT_NULL(contact.last_name);
  TEST_ASSERT_NULL(contact.first_name);
  TEST_ASSERT_NULL(contact.middle_name);
  TEST_ASSERT_NULL(contact.workplace);
  TEST_ASSERT_NULL(contact.position);
  TEST_ASSERT_EQUAL_INT(0, (int)contact.phones.count);
  TEST_ASSERT_EQUAL_INT(0, (int)contact.emails.count);
  TEST_ASSERT_EQUAL_INT(0, (int)contact.social_networks.count);
  TEST_ASSERT_EQUAL_INT(0, (int)contact.messengers.count);

  contact_free(&contact);
}

static void test_free_frees_strings() {
  Contact contact;
  contact_initialize(&contact);

  contact.last_name = malloc(5);
  memcpy(contact.last_name, "Ivan", 5);

  contact_free(&contact);
  TEST_ASSERT_NULL(contact.last_name);

  // Повторный free не должен падать
  contact_free(&contact);
  TEST_PASS();
}

void test_contact_run() {
  test_initialize_is_empty();
  test_free_frees_strings();
}