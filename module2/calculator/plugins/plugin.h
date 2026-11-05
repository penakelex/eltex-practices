#pragma once

#include <command.h>

#define PLUGIN_BINARY(name_text, precedence_value, right_associative_value,    \
                      function_pointer)                                        \
  static const Command PLUGIN_COMMAND = {                                      \
      .name = name_text,                                                       \
      .kind = COMMAND_BINARY,                                                  \
      .precedence = precedence_value,                                          \
      .right_associative = right_associative_value,                            \
      .operation = {.binary = function_pointer},                               \
  };                                                                           \
  const Command *command_export(void) {                                        \
    return &PLUGIN_COMMAND;                                                    \
  }

#define PLUGIN_UNARY(name_text, precedence_value, function_pointer)            \
  static const Command PLUGIN_COMMAND = {                                      \
      .name = name_text,                                                       \
      .kind = COMMAND_UNARY,                                                   \
      .precedence = precedence_value,                                          \
      .right_associative = true,                                               \
      .operation = {.unary = function_pointer},                                \
  };                                                                           \
  const Command *command_export(void) {                                        \
    return &PLUGIN_COMMAND;                                                    \
  }

#define PLUGIN_CONSTANT(name_text, constant_value)                             \
  static const Command PLUGIN_COMMAND = {                                      \
      .name = name_text,                                                       \
      .kind = COMMAND_CONSTANT,                                                \
      .precedence = 0,                                                         \
      .right_associative = false,                                              \
      .operation = {.constant = constant_value},                               \
  };                                                                           \
  const Command *command_export(void) {                                        \
    return &PLUGIN_COMMAND;                                                    \
  }