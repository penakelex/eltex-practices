#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef double (*UnaryFunction)(double);
typedef double (*BinaryFunction)(double, double);

typedef enum { COMMAND_CONSTANT, COMMAND_UNARY, COMMAND_BINARY } CommandKind;

typedef struct {
  const char *name;
  CommandKind kind;
  int precedence;
  bool right_associative;

  union {
    double constant;
    UnaryFunction unary;
    BinaryFunction binary;
  } operation;
} Command;

const Command *command_find(const char *name);
const Command *command_all(size_t *count);
void command_print_help();