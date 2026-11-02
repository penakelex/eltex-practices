#include "unity.h"
#include <circular_buffer.h>

static Message make_message(unsigned int id, unsigned int priority) {
  Message message;
  message.id = id;
  message.priority = priority;
  message.payload[0] = '\0';
  return message;
}

static void test_starts_empty() {
  CircularBuffer buffer;
  circular_buffer_initialize(&buffer);

  TEST_ASSERT_TRUE(circular_buffer_is_empty(&buffer));
  TEST_ASSERT_FALSE(circular_buffer_is_full(&buffer));
  TEST_ASSERT_EQUAL_INT(0, (int)circular_buffer_count(&buffer));
}

static void test_pop_from_empty_returns_false() {
  CircularBuffer buffer;
  circular_buffer_initialize(&buffer);

  Message out;
  TEST_ASSERT_FALSE(circular_buffer_pop(&buffer, &out));
}

static void test_push_and_pop_single() {
  CircularBuffer buffer;
  circular_buffer_initialize(&buffer);

  Message input = make_message(1, 10);
  circular_buffer_push(&buffer, &input);

  Message output;
  TEST_ASSERT_TRUE(circular_buffer_pop(&buffer, &output));
  TEST_ASSERT_EQUAL_UINT(1, output.id);
  TEST_ASSERT_EQUAL_UINT(10, output.priority);
  TEST_ASSERT_TRUE(circular_buffer_is_empty(&buffer));
}

static void test_fifo_order() {
  CircularBuffer buffer;
  circular_buffer_initialize(&buffer);

  for (unsigned int index = 1; index <= 5; ++index) {
    Message message = make_message(index, index);
    circular_buffer_push(&buffer, &message);
  }

  for (unsigned int index = 1; index <= 5; ++index) {
    Message output;
    TEST_ASSERT_TRUE(circular_buffer_pop(&buffer, &output));
    TEST_ASSERT_EQUAL_UINT(index, output.id);
  }

  TEST_ASSERT_TRUE(circular_buffer_is_empty(&buffer));
}

static void test_fill_to_capacity() {
  CircularBuffer buffer;
  circular_buffer_initialize(&buffer);

  for (unsigned int index = 0; index < CIRCULAR_BUFFER_CAPACITY; ++index) {
    Message message = make_message(index, 0);
    circular_buffer_push(&buffer, &message);
  }

  TEST_ASSERT_TRUE(circular_buffer_is_full(&buffer));
  TEST_ASSERT_EQUAL_INT((int)CIRCULAR_BUFFER_CAPACITY,
                        (int)circular_buffer_count(&buffer));
}

static void test_overfill_drops_oldest() {
  CircularBuffer buffer;
  circular_buffer_initialize(&buffer);

  for (unsigned int index = 1; index <= CIRCULAR_BUFFER_CAPACITY; ++index) {
    Message message = make_message(index, 0);
    circular_buffer_push(&buffer, &message);
  }

  Message overflow = make_message(9999, 0);
  circular_buffer_push(&buffer, &overflow);

  TEST_ASSERT_EQUAL_INT((int)CIRCULAR_BUFFER_CAPACITY,
                        (int)circular_buffer_count(&buffer));

  Message output;
  TEST_ASSERT_TRUE(circular_buffer_pop(&buffer, &output));
  TEST_ASSERT_EQUAL_UINT(2, output.id);
}

static void test_ring_wrap() {
  CircularBuffer buffer;
  circular_buffer_initialize(&buffer);

  for (unsigned int index = 1; index <= 100; ++index) {
    Message message = make_message(index, 0);
    circular_buffer_push(&buffer, &message);

    if (index % 2 == 0) {
      Message output;
      TEST_ASSERT_TRUE(circular_buffer_pop(&buffer, &output));
    }
  }

  TEST_ASSERT_EQUAL_INT(50, (int)circular_buffer_count(&buffer));
}

void test_circular_buffer_run() {
  test_starts_empty();
  test_pop_from_empty_returns_false();
  test_push_and_pop_single();
  test_fifo_order();
  test_fill_to_capacity();
  test_overfill_drops_oldest();
  test_ring_wrap();
}