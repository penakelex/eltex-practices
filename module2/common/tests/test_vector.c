#include "unity.h"
#include <vector.h>

static void test_initialize_starts_empty() {
  Vector vector;
  vector_initialize(&vector, sizeof(int), NULL);

  TEST_ASSERT_EQUAL_INT(0, (int)vector.count);
  TEST_ASSERT_EQUAL_INT(0, (int)vector.capacity);
  TEST_ASSERT_NULL(vector.data);

  vector_free(&vector);
}

static void test_push_back_grows() {
  Vector vector;
  vector_initialize(&vector, sizeof(int), NULL);

  const int values[] = {10, 20, 30, 40, 50};

  for (size_t index = 0; index < 5; ++index) {
    TEST_ASSERT_TRUE(vector_push_back(&vector, &values[index]));
  }

  TEST_ASSERT_EQUAL_INT(5, (int)vector.count);
  TEST_ASSERT_GREATER_OR_EQUAL(5, (int)vector.capacity);

  for (size_t index = 0; index < 5; ++index) {
    TEST_ASSERT_EQUAL_INT(values[index], *(int *)vector_at(&vector, index));
  }

  vector_free(&vector);
}

static void test_at_out_of_bounds() {
  Vector vector;
  vector_initialize(&vector, sizeof(int), NULL);

  TEST_ASSERT_NULL(vector_at(&vector, 0));

  const int value = 42;
  vector_push_back(&vector, &value);

  TEST_ASSERT_NULL(vector_at(&vector, 1));
  TEST_ASSERT_NOT_NULL(vector_at(&vector, 0));

  vector_free(&vector);
}

static void test_remove_at() {
  Vector vector;
  vector_initialize(&vector, sizeof(int), NULL);

  const int values[] = {10, 20, 30};

  for (size_t index = 0; index < 3; ++index) {
    vector_push_back(&vector, &values[index]);
  }

  TEST_ASSERT_TRUE(vector_remove_at(&vector, 1));
  TEST_ASSERT_EQUAL_INT(2, (int)vector.count);
  TEST_ASSERT_EQUAL_INT(10, *(int *)vector_at(&vector, 0));
  TEST_ASSERT_EQUAL_INT(30, *(int *)vector_at(&vector, 1));

  TEST_ASSERT_FALSE(vector_remove_at(&vector, 5));

  vector_free(&vector);
}

static int destructor_calls = 0;

static void count_destructor(void *element) {
  (void)element;
  ++destructor_calls;
}

static void test_destructor_called() {
  destructor_calls = 0;

  Vector vector;
  vector_initialize(&vector, sizeof(int), count_destructor);

  const int values[] = {1, 2, 3};

  for (size_t index = 0; index < 3; ++index) {
    vector_push_back(&vector, &values[index]);
  }

  TEST_ASSERT_TRUE(vector_remove_at(&vector, 0));
  TEST_ASSERT_EQUAL_INT(1, destructor_calls);

  vector_clear(&vector);
  TEST_ASSERT_EQUAL_INT(3, destructor_calls);
  TEST_ASSERT_EQUAL_INT(0, (int)vector.count);

  vector_free(&vector);
}

static void test_insert_at() {
  Vector vector;
  vector_initialize(&vector, sizeof(int), NULL);

  const int values[] = {10, 20, 30};
  for (size_t index = 0; index < 3; ++index) {
    TEST_ASSERT_TRUE(vector_push_back(&vector, &values[index]));
  }

  const int first = 5;
  TEST_ASSERT_TRUE(vector_insert_at(&vector, 0, &first));
  TEST_ASSERT_EQUAL_INT(5, *(int *)vector_at(&vector, 0));
  TEST_ASSERT_EQUAL_INT(10, *(int *)vector_at(&vector, 1));

  const int middle = 25;
  TEST_ASSERT_TRUE(vector_insert_at(&vector, 3, &middle));
  TEST_ASSERT_EQUAL_INT(25, *(int *)vector_at(&vector, 3));
  TEST_ASSERT_EQUAL_INT(30, *(int *)vector_at(&vector, 4));

  const int last = 99;
  TEST_ASSERT_TRUE(vector_insert_at(&vector, vector.count, &last));
  TEST_ASSERT_EQUAL_INT(99, *(int *)vector_at(&vector, vector.count - 1));

  TEST_ASSERT_FALSE(vector_insert_at(&vector, vector.count + 1, &last));

  vector_free(&vector);
}

void test_vector_run() {
  test_initialize_starts_empty();
  test_push_back_grows();
  test_at_out_of_bounds();
  test_remove_at();
  test_destructor_called();
  test_insert_at();
}