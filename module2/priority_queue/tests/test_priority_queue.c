#include "unity.h"
#include <priority_queue.h>

static Message make_message(unsigned int id, unsigned int priority) {
  Message message;
  message.id = id;
  message.priority = priority;
  message.payload[0] = '\0';
  return message;
}

static void add_message(PriorityQueue *queue, unsigned int id,
                        unsigned int priority) {
  Message message = make_message(id, priority);
  priority_queue_add(queue, &message);
}

static void test_starts_empty() {
  PriorityQueue queue;
  priority_queue_initialize(&queue);

  TEST_ASSERT_TRUE(priority_queue_is_empty(&queue));
  TEST_ASSERT_EQUAL_INT(0, (int)priority_queue_count(&queue));
}

static void test_add_increases_count() {
  PriorityQueue queue;
  priority_queue_initialize(&queue);

  add_message(&queue, 1, 10);
  add_message(&queue, 2, 20);
  add_message(&queue, 3, 10);

  TEST_ASSERT_EQUAL_INT(3, (int)priority_queue_count(&queue));
  TEST_ASSERT_EQUAL_INT(2, (int)priority_queue_count_with_priority(&queue, 10));
  TEST_ASSERT_EQUAL_INT(1, (int)priority_queue_count_with_priority(&queue, 20));
}

static void test_extract_first_returns_highest_priority() {
  PriorityQueue queue;
  priority_queue_initialize(&queue);

  add_message(&queue, 1, 10);
  add_message(&queue, 2, 200);
  add_message(&queue, 3, 50);

  Message output;
  TEST_ASSERT_TRUE(priority_queue_extract_first(&queue, &output));
  TEST_ASSERT_EQUAL_UINT(200, output.priority);
  TEST_ASSERT_EQUAL_UINT(2, output.id);

  TEST_ASSERT_TRUE(priority_queue_extract_first(&queue, &output));
  TEST_ASSERT_EQUAL_UINT(50, output.priority);

  TEST_ASSERT_TRUE(priority_queue_extract_first(&queue, &output));
  TEST_ASSERT_EQUAL_UINT(10, output.priority);

  TEST_ASSERT_FALSE(priority_queue_extract_first(&queue, &output));
}

static void test_extract_first_fifo_within_priority() {
  PriorityQueue queue;
  priority_queue_initialize(&queue);

  add_message(&queue, 1, 100);
  add_message(&queue, 2, 100);
  add_message(&queue, 3, 100);

  Message output;
  TEST_ASSERT_TRUE(priority_queue_extract_first(&queue, &output));
  TEST_ASSERT_EQUAL_UINT(1, output.id);

  TEST_ASSERT_TRUE(priority_queue_extract_first(&queue, &output));
  TEST_ASSERT_EQUAL_UINT(2, output.id);

  TEST_ASSERT_TRUE(priority_queue_extract_first(&queue, &output));
  TEST_ASSERT_EQUAL_UINT(3, output.id);
}

static void test_extract_with_priority_exact() {
  PriorityQueue queue;
  priority_queue_initialize(&queue);

  add_message(&queue, 1, 10);
  add_message(&queue, 2, 20);
  add_message(&queue, 3, 10);

  Message output;
  TEST_ASSERT_TRUE(priority_queue_extract_with_priority(&queue, 10, &output));
  TEST_ASSERT_EQUAL_UINT(1, output.id);

  TEST_ASSERT_TRUE(priority_queue_extract_with_priority(&queue, 10, &output));
  TEST_ASSERT_EQUAL_UINT(3, output.id);

  TEST_ASSERT_FALSE(priority_queue_extract_with_priority(&queue, 10, &output));
  TEST_ASSERT_FALSE(priority_queue_extract_with_priority(&queue, 30, &output));

  TEST_ASSERT_EQUAL_INT(1, (int)priority_queue_count(&queue));
}

static void test_extract_with_priority_at_least() {
  PriorityQueue queue;
  priority_queue_initialize(&queue);

  add_message(&queue, 1, 10);
  add_message(&queue, 2, 20);
  add_message(&queue, 3, 30);
  add_message(&queue, 4, 40);

  Message output;

  TEST_ASSERT_TRUE(
      priority_queue_extract_with_priority_at_least(&queue, 25, &output));
  TEST_ASSERT_EQUAL_UINT(30, output.priority);

  TEST_ASSERT_TRUE(
      priority_queue_extract_with_priority_at_least(&queue, 25, &output));
  TEST_ASSERT_EQUAL_UINT(20, output.priority);

  TEST_ASSERT_TRUE(
      priority_queue_extract_with_priority_at_least(&queue, 25, &output));
  TEST_ASSERT_EQUAL_UINT(40, output.priority);

  TEST_ASSERT_TRUE(
      priority_queue_extract_with_priority_at_least(&queue, 25, &output));
  TEST_ASSERT_EQUAL_UINT(10, output.priority);

  TEST_ASSERT_FALSE(
      priority_queue_extract_with_priority_at_least(&queue, 25, &output));
  TEST_ASSERT_EQUAL_INT(0, (int)priority_queue_count(&queue));
}

static void test_extract_with_priority_at_least_includes_threshold() {
  PriorityQueue queue;
  priority_queue_initialize(&queue);

  add_message(&queue, 1, 100);
  add_message(&queue, 2, 50);
  add_message(&queue, 3, 25);

  Message output;

  TEST_ASSERT_TRUE(
      priority_queue_extract_with_priority_at_least(&queue, 50, &output));
  TEST_ASSERT_EQUAL_UINT(50, output.priority);

  TEST_ASSERT_TRUE(
      priority_queue_extract_with_priority_at_least(&queue, 50, &output));
  TEST_ASSERT_EQUAL_UINT(25, output.priority);

  TEST_ASSERT_TRUE(
      priority_queue_extract_with_priority_at_least(&queue, 50, &output));
  TEST_ASSERT_EQUAL_UINT(100, output.priority);

  TEST_ASSERT_FALSE(
      priority_queue_extract_with_priority_at_least(&queue, 50, &output));
}

static void test_overflow_drops_oldest_at_that_level() {
  PriorityQueue queue;
  priority_queue_initialize(&queue);

  for (unsigned int index = 0; index < CIRCULAR_BUFFER_CAPACITY; ++index) {
    add_message(&queue, index, 100);
  }

  add_message(&queue, 9999, 100);
  TEST_ASSERT_EQUAL_INT((int)CIRCULAR_BUFFER_CAPACITY,
                        (int)priority_queue_count_with_priority(&queue, 100));

  Message output;
  TEST_ASSERT_TRUE(priority_queue_extract_with_priority(&queue, 100, &output));
  TEST_ASSERT_EQUAL_UINT(1, output.id);
}

static void test_total_count_consistent() {
  PriorityQueue queue;
  priority_queue_initialize(&queue);

  for (unsigned int index = 0; index < 20; ++index) {
    add_message(&queue, index, index * 10);
  }

  TEST_ASSERT_EQUAL_INT(20, (int)priority_queue_count(&queue));

  Message output;
  while (priority_queue_extract_first(&queue, &output)) {
  }

  TEST_ASSERT_EQUAL_INT(0, (int)priority_queue_count(&queue));
  TEST_ASSERT_TRUE(priority_queue_is_empty(&queue));
}

void test_priority_queue_run() {
  test_starts_empty();
  test_add_increases_count();
  test_extract_first_returns_highest_priority();
  test_extract_first_fifo_within_priority();
  test_extract_with_priority_exact();
  test_extract_with_priority_at_least();
  test_extract_with_priority_at_least_includes_threshold();
  test_overflow_drops_oldest_at_that_level();
  test_total_count_consistent();
}