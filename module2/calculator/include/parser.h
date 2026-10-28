#pragma once

#include <ast.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
  PARSER_OK,
  PARSER_ERROR_SYNTAX,
  PARSER_ERROR_UNKNOWN_COMMAND,
  PARSER_ERROR_MEMORY,
} ParserStatus;

ParserStatus parser_parse(const char *expression, AstNode **root,
                          const char **error_position);