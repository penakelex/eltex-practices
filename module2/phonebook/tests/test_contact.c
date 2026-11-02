#include "unity.h"
#include <contact.h>
#include <stdlib.h>
#include <string.h>
#include <string_utils.h>

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

  contact_free(&contact);
  TEST_PASS();
}

static void test_compare_by_name() {
  Contact first;
  Contact second;
  Contact same_as_first;
  contact_initialize(&first);
  contact_initialize(&second);
  contact_initialize(&same_as_first);

  first.last_name = duplicate_string("Ivanov");
  first.first_name = duplicate_string("Ivan");

  second.last_name = duplicate_string("Ivanov");
  second.first_name = duplicate_string("Petr");

  same_as_first.last_name = duplicate_string("Ivanov");
  same_as_first.first_name = duplicate_string("Ivan");

  TEST_ASSERT_LESS_THAN(0, contact_compare(&first, &second));
  TEST_ASSERT_GREATER_THAN(0, contact_compare(&second, &first));
  TEST_ASSERT_EQUAL_INT(0, contact_compare(&first, &same_as_first));

  contact_free(&first);
  contact_free(&second);
  contact_free(&same_as_first);
}

static void test_compare_handles_null_fields() {
  Contact filled;
  Contact empty;
  contact_initialize(&filled);
  contact_initialize(&empty);

  filled.last_name = duplicate_string("A");
  filled.first_name = duplicate_string("A");

  TEST_ASSERT_GREATER_THAN(0, contact_compare(&filled, &empty));
  TEST_ASSERT_LESS_THAN(0, contact_compare(&empty, &filled));
  TEST_ASSERT_EQUAL_INT(0, contact_compare(&empty, &empty));

  contact_free(&filled);
  contact_free(&empty);
}

void test_contact_run() {
  test_initialize_is_empty();
  test_free_frees_strings();
  test_compare_by_name();
  test_compare_handles_null_fields();
}