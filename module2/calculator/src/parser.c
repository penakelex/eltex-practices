#include <command.h>
#include <ctype.h>
#include <parser.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

constexpr size_t IDENTIFIER_BUFFER_SIZE = 32;

typedef struct {
  const char *cursor;
  const char *error_position;
} Parser;

static void parser_skip_whitespace(Parser *parser) {
  while (*parser->cursor == ' ' || *parser->cursor == '\t') {
    ++parser->cursor;
  }
}

static bool parser_is_identifier_start(const char character) {
  return isalpha((unsigned char)character) || character == '_';
}

static bool parser_is_identifier_char(const char character) {
  return isalnum((unsigned char)character) || character == '_';
}

static AstNode *parse_expression_precedence(Parser *parser, int min_precedence);

static AstNode *parse_primary(Parser *parser) {
  parser_skip_whitespace(parser);

  const char character = *parser->cursor;

  if (isdigit((unsigned char)character) || character == '.') {
    char *end_pointer = nullptr;
    const double value = strtod(parser->cursor, &end_pointer);

    if (end_pointer == parser->cursor) {
      parser->error_position = parser->cursor;
      return nullptr;
    }

    parser->cursor = end_pointer;
    return ast_number(value);
  }

  if (parser_is_identifier_start(character)) {
    char buffer[IDENTIFIER_BUFFER_SIZE];
    size_t length = 0;

    while (parser_is_identifier_char(parser->cursor[length])) {
      if (length + 1 >= sizeof buffer) {
        parser->error_position = parser->cursor;
        return nullptr;
      }

      buffer[length] = parser->cursor[length];
      ++length;
    }

    buffer[length] = '\0';
    parser->cursor += length;

    const Command *command = command_find(buffer);

    if (command == nullptr) {
      parser->error_position = parser->cursor - length;
      return nullptr;
    }

    if (command->kind == COMMAND_CONSTANT) {
      return ast_number(command->operation.constant);
    }

    if (command->kind == COMMAND_UNARY) {
      parser_skip_whitespace(parser);

      if (*parser->cursor != '(') {
        parser->error_position = parser->cursor;
        return nullptr;
      }

      ++parser->cursor;

      AstNode *operand = parse_expression_precedence(parser, 0);

      if (operand == nullptr) {
        return nullptr;
      }

      parser_skip_whitespace(parser);

      if (*parser->cursor != ')') {
        parser->error_position = parser->cursor;
        ast_free(operand);
        return nullptr;
      }

      ++parser->cursor;
      return ast_unary(command->operation.unary, operand);
    }

    parser->error_position = parser->cursor - length;
    return nullptr;
  }

  if (character == '(') {
    ++parser->cursor;

    AstNode *inner = parse_expression_precedence(parser, 0);

    if (inner == nullptr) {
      return nullptr;
    }

    parser_skip_whitespace(parser);

    if (*parser->cursor != ')') {
      parser->error_position = parser->cursor;
      ast_free(inner);
      return nullptr;
    }

    ++parser->cursor;
    return inner;
  }

  parser->error_position = parser->cursor;
  return nullptr;
}

static AstNode *parse_prefix(Parser *parser) {
  parser_skip_whitespace(parser);

  if (*parser->cursor == '-') {
    ++parser->cursor;

    AstNode *operand = parse_prefix(parser);

    if (operand == nullptr) {
      return nullptr;
    }

    const Command *negate = command_find("neg");

    if (negate == nullptr) {
      ast_free(operand);
      return nullptr;
    }

    return ast_unary(negate->operation.unary, operand);
  }

  if (*parser->cursor == '+') {
    ++parser->cursor;
    return parse_prefix(parser);
  }

  return parse_primary(parser);
}

static const Command *parser_read_binary_operator(Parser *parser) {
  parser_skip_whitespace(parser);

  size_t count = 0;
  const Command *all = command_all(&count);
  const Command *best = nullptr;
  size_t best_length = 0;

  for (size_t index = 0; index < count; ++index) {
    if (all[index].kind != COMMAND_BINARY) {
      continue;
    }

    const size_t length = strlen(all[index].name);

    if (length <= best_length) {
      continue;
    }

    if (strncmp(parser->cursor, all[index].name, length) != 0) {
      continue;
    }

    if (isalpha((unsigned char)all[index].name[0])) {
      const char next = parser->cursor[length];

      if (parser_is_identifier_char(next)) {
        continue;
      }
    }

    best = &all[index];
    best_length = length;
  }

  return best;
}

static AstNode *parse_expression_precedence(Parser *parser,
                                            const int min_precedence) {
  AstNode *left = parse_prefix(parser);

  if (left == nullptr) {
    return nullptr;
  }

  while (true) {
    const Command *operation = parser_read_binary_operator(parser);

    if (operation == nullptr) {
      return left;
    }

    if (operation->precedence < min_precedence) {
      return left;
    }

    parser->cursor += strlen(operation->name);

    const int next_precedence = operation->right_associative
                                    ? operation->precedence
                                    : operation->precedence + 1;

    AstNode *right = parse_expression_precedence(parser, next_precedence);

    if (right == nullptr) {
      ast_free(left);
      return nullptr;
    }

    AstNode *combined = ast_binary(operation->operation.binary, left, right);

    if (combined == nullptr) {
      ast_free(left);
      ast_free(right);
      return nullptr;
    }

    left = combined;
  }
}

ParserStatus parser_parse(const char *expression, AstNode **root,
                          const char **error_position) {
  if (expression == nullptr || root == nullptr) {
    return PARSER_ERROR_SYNTAX;
  }

  *root = nullptr;

  if (error_position != nullptr) {
    *error_position = nullptr;
  }

  Parser parser = {
      .cursor = expression,
      .error_position = nullptr,
  };

  AstNode *node = parse_expression_precedence(&parser, 0);

  if (node == nullptr) {
    if (error_position != nullptr) {
      *error_position = parser.error_position;
    }

    return PARSER_ERROR_SYNTAX;
  }

  parser_skip_whitespace(&parser);

  if (*parser.cursor != '\0') {
    if (error_position != nullptr) {
      *error_position = parser.cursor;
    }

    ast_free(node);
    return PARSER_ERROR_SYNTAX;
  }

  *root = node;
  return PARSER_OK;
}