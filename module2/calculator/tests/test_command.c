#include "unity.h"
#include <command.h>
#include <math.h>
#include <string.h>

constexpr double TEST_PI = 3.14159265358979323846;
constexpr double TEST_E = 2.71828182845904523536;

static void test_find_constants() {
  const Command *pi = command_find("pi");
  TEST_ASSERT_NOT_NULL(pi);
  TEST_ASSERT_EQUAL_INT(COMMAND_CONSTANT, pi->kind);
  TEST_ASSERT_EQUAL_DOUBLE(TEST_PI, pi->operation.constant);

  const Command *e = command_find("e");
  TEST_ASSERT_NOT_NULL(e);
  TEST_ASSERT_EQUAL_INT(COMMAND_CONSTANT, e->kind);
  TEST_ASSERT_EQUAL_DOUBLE(TEST_E, e->operation.constant);
}

static void test_find_binary_ops() {
  const Command *add = command_find("+");
  TEST_ASSERT_NOT_NULL(add);
  TEST_ASSERT_EQUAL_INT(COMMAND_BINARY, add->kind);
  TEST_ASSERT_EQUAL_INT(1, add->precedence);
  TEST_ASSERT_FALSE(add->right_associative);

  const Command *subtract = command_find("-");
  TEST_ASSERT_NOT_NULL(subtract);
  TEST_ASSERT_EQUAL_INT(COMMAND_BINARY, subtract->kind);
  TEST_ASSERT_EQUAL_INT(1, subtract->precedence);

  const Command *multiply = command_find("*");
  TEST_ASSERT_NOT_NULL(multiply);
  TEST_ASSERT_EQUAL_INT(2, multiply->precedence);

  const Command *divide = command_find("/");
  TEST_ASSERT_NOT_NULL(divide);
  TEST_ASSERT_EQUAL_INT(2, divide->precedence);

  const Command *power = command_find("^");
  TEST_ASSERT_NOT_NULL(power);
  TEST_ASSERT_EQUAL_INT(COMMAND_BINARY, power->kind);
  TEST_ASSERT_EQUAL_INT(3, power->precedence);
  TEST_ASSERT_TRUE(power->right_associative);
}

static void test_find_unary() {
  const Command *sqrt_command = command_find("sqrt");
  TEST_ASSERT_NOT_NULL(sqrt_command);
  TEST_ASSERT_EQUAL_INT(COMMAND_UNARY, sqrt_command->kind);
  TEST_ASSERT_EQUAL_DOUBLE(4.0, sqrt_command->operation.unary(16.0));

  const Command *neg_command = command_find("neg");
  TEST_ASSERT_NOT_NULL(neg_command);
  TEST_ASSERT_EQUAL_DOUBLE(-5.0, neg_command->operation.unary(5.0));

  const Command *abs_command = command_find("abs");
  TEST_ASSERT_NOT_NULL(abs_command);
  TEST_ASSERT_EQUAL_DOUBLE(3.0, abs_command->operation.unary(-3.0));
}

static void test_find_unknown() {
  TEST_ASSERT_NULL(command_find("nope"));
  TEST_ASSERT_NULL(command_find(""));
  TEST_ASSERT_NULL(command_find("foo"));
}

static void test_all_returns_something() {
  size_t count = 0;
  const Command *all = command_all(&count);

  TEST_ASSERT_NOT_NULL(all);
  TEST_ASSERT_GREATER_THAN(0, (int)count);

  bool saw_pi = false;
  bool saw_add = false;
  bool saw_sqrt = false;

  for (size_t index = 0; index < count; ++index) {
    if (strcmp(all[index].name, "pi") == 0) {
      saw_pi = true;
    }

    if (strcmp(all[index].name, "+") == 0) {
      saw_add = true;
    }

    if (strcmp(all[index].name, "sqrt") == 0) {
      saw_sqrt = true;
    }
  }

  TEST_ASSERT_TRUE(saw_pi);
  TEST_ASSERT_TRUE(saw_add);
  TEST_ASSERT_TRUE(saw_sqrt);
}

void test_command_run() {
  test_find_constants();
  test_find_binary_ops();
  test_find_unary();
  test_find_unknown();
  test_all_returns_something();
}