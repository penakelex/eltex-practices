#include "unity.h"
#include <contact.h>
#include <phonebook.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string_utils.h>

static void make_contact(Contact *contact, const char *last_name,
                         const char *first_name) {
  contact_initialize(contact);
  contact->last_name = duplicate_string(last_name);
  contact->first_name = duplicate_string(first_name);
}

static void test_initialize_is_empty() {
  Phonebook phonebook;
  TEST_ASSERT_TRUE(phonebook_initialize(&phonebook, "array"));
  TEST_ASSERT_EQUAL_INT(0, (int)phonebook_count(&phonebook));
  phonebook_free(&phonebook);
}

static void test_initialize_unknown_collection() {
  Phonebook phonebook;
  TEST_ASSERT_FALSE(phonebook_initialize(&phonebook, "unknown"));
}

static void run_add_and_remove(const char *name) {
  Phonebook phonebook;
  TEST_ASSERT_TRUE(phonebook_initialize(&phonebook, name));

  Contact contact;
  make_contact(&contact, "Ivanov", "Ivan");
  TEST_ASSERT_TRUE(phonebook_add(&phonebook, &contact));
  TEST_ASSERT_EQUAL_INT(1, (int)phonebook_count(&phonebook));
  TEST_ASSERT_NULL(contact.last_name);

  TEST_ASSERT_TRUE(phonebook_remove(&phonebook, 0));
  TEST_ASSERT_EQUAL_INT(0, (int)phonebook_count(&phonebook));
  TEST_ASSERT_FALSE(phonebook_remove(&phonebook, 5));

  phonebook_free(&phonebook);
}

static void run_sorted_insertion(const char *name) {
  Phonebook phonebook;
  TEST_ASSERT_TRUE(phonebook_initialize(&phonebook, name));

  const char *last_names[] = {"Zaharov", "Andreev", "Mironov"};

  for (size_t index = 0; index < 3; ++index) {
    Contact contact;
    make_contact(&contact, last_names[index], "Name");
    TEST_ASSERT_TRUE(phonebook_add(&phonebook, &contact));
  }

  TEST_ASSERT_EQUAL_STRING("Andreev", phonebook_at(&phonebook, 0)->last_name);
  TEST_ASSERT_EQUAL_STRING("Mironov", phonebook_at(&phonebook, 1)->last_name);
  TEST_ASSERT_EQUAL_STRING("Zaharov", phonebook_at(&phonebook, 2)->last_name);

  phonebook_free(&phonebook);
}

static void run_reorder(const char *name) {
  Phonebook phonebook;
  TEST_ASSERT_TRUE(phonebook_initialize(&phonebook, name));

  Contact contact;
  make_contact(&contact, "Andreev", "Andrey");
  TEST_ASSERT_TRUE(phonebook_add(&phonebook, &contact));

  Contact *stored = phonebook_at(&phonebook, 0);
  free(stored->last_name);
  stored->last_name = duplicate_string("Zaharov");

  phonebook_reorder(&phonebook);

  TEST_ASSERT_EQUAL_STRING("Zaharov", phonebook_at(&phonebook, 0)->last_name);

  phonebook_free(&phonebook);
}

static size_t visited_count = 0;

static void count_visitor(const Contact *contact, size_t index, void *context) {
  (void)contact;
  (void)context;
  TEST_ASSERT_EQUAL_INT((int)visited_count, (int)index);
  ++visited_count;
}

static void run_dump(const char *name) {
  Phonebook phonebook;
  TEST_ASSERT_TRUE(phonebook_initialize(&phonebook, name));

  const char *last_names[] = {"Andreev", "Mironov", "Zaharov"};

  for (size_t index = 0; index < 3; ++index) {
    Contact contact;
    make_contact(&contact, last_names[index], "Name");
    TEST_ASSERT_TRUE(phonebook_add(&phonebook, &contact));
  }

  visited_count = 0;
  phonebook_dump(&phonebook, count_visitor, nullptr);
  TEST_ASSERT_EQUAL_INT(3, (int)visited_count);

  phonebook_free(&phonebook);
}

static void test_hierarchical_flag() {
  Phonebook phonebook;

  TEST_ASSERT_TRUE(phonebook_initialize(&phonebook, "array"));
  TEST_ASSERT_FALSE(phonebook_is_hierarchical(&phonebook));
  phonebook_free(&phonebook);

  TEST_ASSERT_TRUE(phonebook_initialize(&phonebook, "list"));
  TEST_ASSERT_FALSE(phonebook_is_hierarchical(&phonebook));
  phonebook_free(&phonebook);

  TEST_ASSERT_TRUE(phonebook_initialize(&phonebook, "tree"));
  TEST_ASSERT_TRUE(phonebook_is_hierarchical(&phonebook));
  phonebook_free(&phonebook);
}

static size_t maximum_prefix_length = 0;

static void max_prefix_visitor(const Contact *contact, const char *prefix,
                               void *context) {
  (void)contact;
  (void)context;
  const size_t length = strlen(prefix);

  if (length > maximum_prefix_length) {
    maximum_prefix_length = length;
  }
}

static void test_tree_remains_balanced() {
  Phonebook phonebook;
  TEST_ASSERT_TRUE(phonebook_initialize(&phonebook, "tree"));

  char buffer[16];

  for (int index = 0; index < 100; ++index) {
    Contact contact;
    contact_initialize(&contact);
    snprintf(buffer, sizeof buffer, "A%03d", index);
    contact.last_name = duplicate_string(buffer);
    contact.first_name = duplicate_string("Name");
    TEST_ASSERT_TRUE(phonebook_add(&phonebook, &contact));
  }

  maximum_prefix_length = 0;
  phonebook_dump_tree(&phonebook, max_prefix_visitor, nullptr);

  TEST_ASSERT_LESS_THAN(80, (int)maximum_prefix_length);

  for (int index = 0; index < 100; ++index) {
    Contact *contact = phonebook_at(&phonebook, (size_t)index);
    snprintf(buffer, sizeof buffer, "A%03d", index);
    TEST_ASSERT_EQUAL_STRING(buffer, contact->last_name);
  }

  phonebook_free(&phonebook);
}

static size_t tree_visit_count = 0;

static void count_tree_visitor(const Contact *contact, const char *prefix,
                               void *context) {
  (void)contact;
  (void)prefix;
  (void)context;
  ++tree_visit_count;
}

static void test_tree_dump_tree_visits_all() {
  Phonebook phonebook;
  TEST_ASSERT_TRUE(phonebook_initialize(&phonebook, "tree"));

  for (int index = 0; index < 10; ++index) {
    Contact contact;
    contact_initialize(&contact);
    char buffer[16];
    snprintf(buffer, sizeof buffer, "Last%02d", index);
    contact.last_name = duplicate_string(buffer);
    contact.first_name = duplicate_string("Name");
    TEST_ASSERT_TRUE(phonebook_add(&phonebook, &contact));
  }

  tree_visit_count = 0;
  phonebook_dump_tree(&phonebook, count_tree_visitor, nullptr);
  TEST_ASSERT_EQUAL_INT(10, (int)tree_visit_count);

  phonebook_free(&phonebook);
}

static void test_array_add_and_remove() {
  run_add_and_remove("array");
}

static void test_list_add_and_remove() {
  run_add_and_remove("list");
}

static void test_tree_add_and_remove() {
  run_add_and_remove("tree");
}

static void test_array_sorted_insertion() {
  run_sorted_insertion("array");
}

static void test_list_sorted_insertion() {
  run_sorted_insertion("list");
}

static void test_tree_sorted_insertion() {
  run_sorted_insertion("tree");
}

static void test_array_reorder() {
  run_reorder("array");
}

static void test_list_reorder() {
  run_reorder("list");
}

static void test_tree_reorder() {
  run_reorder("tree");
}

static void test_array_dump() {
  run_dump("array");
}

static void test_list_dump() {
  run_dump("list");
}

static void test_tree_dump() {
  run_dump("tree");
}

void test_phonebook_run() {
  test_initialize_is_empty();
  test_initialize_unknown_collection();
  test_array_add_and_remove();
  test_list_add_and_remove();
  test_tree_add_and_remove();
  test_array_sorted_insertion();
  test_list_sorted_insertion();
  test_tree_sorted_insertion();
  test_array_reorder();
  test_list_reorder();
  test_tree_reorder();
  test_array_dump();
  test_list_dump();
  test_tree_dump();
  test_hierarchical_flag();
  test_tree_remains_balanced();
  test_tree_dump_tree_visits_all();
}