#pragma once

typedef enum {
  CALCULATOR_OK,
  CALCULATOR_ERROR_PARSE,
  CALCULATOR_ERROR_MEMORY,
  CALCULATOR_ERROR_DOMAIN,
} CalculatorStatus;

CalculatorStatus calculator_evaluate(const char *expression, double *result,
                                     const char **error_position);