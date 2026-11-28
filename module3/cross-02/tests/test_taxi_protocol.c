#define _POSIX_C_SOURCE 202405L

#include "taxi_protocol.h"

#include <unistd.h>

#include "unity.h"

static int pipe_fd[2];

static void open_pipe() {
  TEST_ASSERT_EQUAL_INT(0, pipe(pipe_fd));
}

static void close_pipe() {
  close(pipe_fd[0]);
  close(pipe_fd[1]);
}

void test_command_roundtrip() {
  open_pipe();

  const TaxiCommandMessage sent = {.type = TAXI_COMMAND_TASK, .seconds = 42};
  TEST_ASSERT_TRUE(taxi_protocol_write_command(pipe_fd[1], &sent));

  TaxiCommandMessage received = {};
  TEST_ASSERT_TRUE(taxi_protocol_read_command(pipe_fd[0], &received));
  TEST_ASSERT_EQUAL_INT32(TAXI_COMMAND_TASK, received.type);
  TEST_ASSERT_EQUAL_UINT32(42, received.seconds);

  close_pipe();
}

void test_event_roundtrip() {
  open_pipe();

  const TaxiEventMessage sent = {
      .type = TAXI_EVENT_STATUS_BUSY, .pid = 1234, .seconds = 7};
  TEST_ASSERT_TRUE(taxi_protocol_write_event(pipe_fd[1], &sent));

  TaxiEventMessage received = {};
  TEST_ASSERT_TRUE(taxi_protocol_read_event(pipe_fd[0], &received));
  TEST_ASSERT_EQUAL_INT32(TAXI_EVENT_STATUS_BUSY, received.type);
  TEST_ASSERT_EQUAL_INT32(1234, received.pid);
  TEST_ASSERT_EQUAL_UINT32(7, received.seconds);

  close_pipe();
}

void test_read_fails_on_closed_pipe() {
  open_pipe();
  close(pipe_fd[1]);

  TaxiCommandMessage message = {};
  TEST_ASSERT_FALSE(taxi_protocol_read_command(pipe_fd[0], &message));

  close(pipe_fd[0]);
}

void test_invalid_arguments_rejected() {
  TaxiCommandMessage command = {};
  TaxiEventMessage event = {};

  TEST_ASSERT_FALSE(taxi_protocol_write_command(-1, &command));
  TEST_ASSERT_FALSE(taxi_protocol_write_command(0, nullptr));
  TEST_ASSERT_FALSE(taxi_protocol_read_command(-1, &command));
  TEST_ASSERT_FALSE(taxi_protocol_read_command(0, nullptr));
  TEST_ASSERT_FALSE(taxi_protocol_write_event(-1, &event));
  TEST_ASSERT_FALSE(taxi_protocol_write_event(0, nullptr));
  TEST_ASSERT_FALSE(taxi_protocol_read_event(-1, &event));
  TEST_ASSERT_FALSE(taxi_protocol_read_event(0, nullptr));
}

void test_taxi_protocol_run() {
  RUN_TEST(test_command_roundtrip);
  RUN_TEST(test_event_roundtrip);
  RUN_TEST(test_read_fails_on_closed_pipe);
  RUN_TEST(test_invalid_arguments_rejected);
}
