#pragma once

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

bool command_register(const Command *command);
void command_reset(void);
const Command *command_find(const char *name);
const Command *const *command_all(size_t *count);