#include <ast.h>
#include <calculator.h>
#include <math.h>
#include <parser.h>

CalculatorStatus calculator_evaluate(const char *expression, double *result,
                                     const char **error_position) {
  if (expression == nullptr || result == nullptr) {
    return CALCULATOR_ERROR_PARSE;
  }

  if (error_position != nullptr) {
    *error_position = nullptr;
  }

  AstNode *root = nullptr;
  const char *parse_error = nullptr;
  const ParserStatus status = parser_parse(expression, &root, &parse_error);

  if (status != PARSER_OK) {
    if (error_position != nullptr) {
      *error_position = parse_error;
    }

    return CALCULATOR_ERROR_PARSE;
  }

  double value = 0.0;

  if (!ast_evaluate(root, &value)) {
    ast_free(root);
    return CALCULATOR_ERROR_MEMORY;
  }

  ast_free(root);

  if (isnan(value)) {
    return CALCULATOR_ERROR_DOMAIN;
  }

  *result = value;
  return CALCULATOR_OK;
}