#include <command.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
constexpr double M_PI = 3.14159265358979323846;
#endif

#ifndef M_E
constexpr double M_E = 2.71828182845904523536;
#endif

static double command_add(double left, double right) {
  return left + right;
}

static double command_subtract(double left, double right) {
  return left - right;
}

static double command_multiply(double left, double right) {
  return left * right;
}

static double command_divide(double left, double right) {
  if (right == 0.0) {
    return NAN;
  }

  return left / right;
}

static double command_power(double left, double right) {
  return pow(left, right);
}

static double command_negate(double value) {
  return -value;
}

static const Command COMMANDS[] = {
    {"pi", COMMAND_CONSTANT, 0, false, {.constant = M_PI}},
    {"e", COMMAND_CONSTANT, 0, false, {.constant = M_E}},

    {"+", COMMAND_BINARY, 1, false, {.binary = command_add}},
    {"-", COMMAND_BINARY, 1, false, {.binary = command_subtract}},
    {"*", COMMAND_BINARY, 2, false, {.binary = command_multiply}},
    {"/", COMMAND_BINARY, 2, false, {.binary = command_divide}},
    {"^", COMMAND_BINARY, 3, true, {.binary = command_power}},

    {"neg", COMMAND_UNARY, 4, true, {.unary = command_negate}},
    {"sqrt", COMMAND_UNARY, 4, true, {.unary = sqrt}},
    {"sin", COMMAND_UNARY, 4, true, {.unary = sin}},
    {"cos", COMMAND_UNARY, 4, true, {.unary = cos}},
    {"tan", COMMAND_UNARY, 4, true, {.unary = tan}},
    {"asin", COMMAND_UNARY, 4, true, {.unary = asin}},
    {"acos", COMMAND_UNARY, 4, true, {.unary = acos}},
    {"atan", COMMAND_UNARY, 4, true, {.unary = atan}},
    {"ln", COMMAND_UNARY, 4, true, {.unary = log}},
    {"log", COMMAND_UNARY, 4, true, {.unary = log10}},
    {"exp", COMMAND_UNARY, 4, true, {.unary = exp}},
    {"abs", COMMAND_UNARY, 4, true, {.unary = fabs}},
};

constexpr size_t COMMAND_COUNT = sizeof(COMMANDS) / sizeof(COMMANDS[0]);

const Command *command_find(const char *name) {
  for (size_t index = 0; index < COMMAND_COUNT; ++index) {
    if (strcmp(COMMANDS[index].name, name) == 0) {
      return &COMMANDS[index];
    }
  }

  return nullptr;
}

const Command *command_all(size_t *count) {
  if (count != nullptr) {
    *count = COMMAND_COUNT;
  }

  return COMMANDS;
}