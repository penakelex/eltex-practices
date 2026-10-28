#include "unity.h"
#include <contact.h>
#include <phonebook.h>

static void test_initialize_is_empty() {
  Phonebook phonebook;
  phonebook_initialize(&phonebook);

  TEST_ASSERT_EQUAL_INT(0, (int)phonebook.contacts.count);

  phonebook_free(&phonebook);
}

static void test_add_and_remove() {
  Phonebook phonebook;
  phonebook_initialize(&phonebook);

  Contact contact;
  contact_initialize(&contact);

  TEST_ASSERT_TRUE(phonebook_add(&phonebook, &contact));
  TEST_ASSERT_EQUAL_INT(1, (int)phonebook.contacts.count);

  // После add контакт переинициализирован (отдан вектору)
  TEST_ASSERT_NULL(contact.last_name);

  TEST_ASSERT_TRUE(phonebook_remove(&phonebook, 0));
  TEST_ASSERT_EQUAL_INT(0, (int)phonebook.contacts.count);

  TEST_ASSERT_FALSE(phonebook_remove(&phonebook, 5));

  phonebook_free(&phonebook);
}

static void test_add_multiple() {
  Phonebook phonebook;
  phonebook_initialize(&phonebook);

  for (int index = 0; index < 5; ++index) {
    Contact contact;
    contact_initialize(&contact);
    TEST_ASSERT_TRUE(phonebook_add(&phonebook, &contact));
  }

  TEST_ASSERT_EQUAL_INT(5, (int)phonebook.contacts.count);

  phonebook_free(&phonebook);
}

void test_phonebook_run() {
  test_initialize_is_empty();
  test_add_and_remove();
  test_add_multiple();
}