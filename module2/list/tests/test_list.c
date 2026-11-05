#include "unity.h"
#include <list.h>
#include <stdlib.h>

static int compare_ints(const void *left, const void *right) {
  const int a = *(const int *)left;
  const int b = *(const int *)right;

  if (a < b) {
    return -1;
  }

  if (a > b) {
    return 1;
  }

  return 0;
}

static void test_empty_list() {
  List *list = list_create(sizeof(int), compare_ints, nullptr);
  TEST_ASSERT_NOT_NULL(list);
  TEST_ASSERT_EQUAL_INT(0, (int)list_count(list));
  TEST_ASSERT_NULL(list_at(list, 0));
  list_destroy(list);
}

static void test_add_keeps_order() {
  List *list = list_create(sizeof(int), compare_ints, nullptr);

  const int values[] = {30, 10, 20, 40, 5};

  for (size_t index = 0; index < 5; ++index) {
    TEST_ASSERT_TRUE(list_add(list, &values[index]));
  }

  TEST_ASSERT_EQUAL_INT(5, (int)list_count(list));
  TEST_ASSERT_EQUAL_INT(5, *(int *)list_at(list, 0));
  TEST_ASSERT_EQUAL_INT(10, *(int *)list_at(list, 1));
  TEST_ASSERT_EQUAL_INT(20, *(int *)list_at(list, 2));
  TEST_ASSERT_EQUAL_INT(30, *(int *)list_at(list, 3));
  TEST_ASSERT_EQUAL_INT(40, *(int *)list_at(list, 4));

  list_destroy(list);
}

static void test_remove_at() {
  List *list = list_create(sizeof(int), compare_ints, nullptr);

  const int values[] = {10, 20, 30, 40};

  for (size_t index = 0; index < 4; ++index) {
    list_add(list, &values[index]);
  }

  TEST_ASSERT_TRUE(list_remove_at(list, 1));
  TEST_ASSERT_EQUAL_INT(3, (int)list_count(list));
  TEST_ASSERT_EQUAL_INT(10, *(int *)list_at(list, 0));
  TEST_ASSERT_EQUAL_INT(30, *(int *)list_at(list, 1));
  TEST_ASSERT_EQUAL_INT(40, *(int *)list_at(list, 2));

  TEST_ASSERT_FALSE(list_remove_at(list, 10));

  list_destroy(list);
}

static int destructor_calls = 0;

static void count_destructor(void *element) {
  (void)element;
  ++destructor_calls;
}

static void test_destructor_called() {
  destructor_calls = 0;
  List *list = list_create(sizeof(int), compare_ints, count_destructor);

  const int values[] = {1, 2, 3};

  for (size_t index = 0; index < 3; ++index) {
    list_add(list, &values[index]);
  }

  TEST_ASSERT_TRUE(list_remove_at(list, 0));
  TEST_ASSERT_EQUAL_INT(1, destructor_calls);

  list_destroy(list);
  TEST_ASSERT_EQUAL_INT(3, destructor_calls);
}

static void test_reorder_element() {
  List *list = list_create(sizeof(int), compare_ints, nullptr);

  const int values[] = {30, 10, 20};

  for (size_t index = 0; index < 3; ++index) {
    list_add(list, &values[index]);
  }

  int *first = list_at(list, 0);
  *first = 99;

  TEST_ASSERT_TRUE(list_reorder_element(list, 0));

  TEST_ASSERT_EQUAL_INT(20, *(int *)list_at(list, 0));
  TEST_ASSERT_EQUAL_INT(30, *(int *)list_at(list, 1));
  TEST_ASSERT_EQUAL_INT(99, *(int *)list_at(list, 2));

  TEST_ASSERT_FALSE(list_reorder_element(list, 10));

  list_destroy(list);
}

static size_t visited_count = 0;

static void count_visitor(const void *element, const size_t index,
                          void *context) {
  (void)element;
  (void)context;
  TEST_ASSERT_EQUAL_INT((int)visited_count, (int)index);
  ++visited_count;
}

static void test_for_each() {
  List *list = list_create(sizeof(int), compare_ints, nullptr);

  const int values[] = {1, 2, 3, 4, 5};

  for (size_t index = 0; index < 5; ++index) {
    list_add(list, &values[index]);
  }

  visited_count = 0;
  list_for_each(list, count_visitor, nullptr);
  TEST_ASSERT_EQUAL_INT(5, (int)visited_count);

  list_destroy(list);
}

void test_list_run() {
  test_empty_list();
  test_add_keeps_order();
  test_remove_at();
  test_destructor_called();
  test_reorder_element();
  test_for_each();
}