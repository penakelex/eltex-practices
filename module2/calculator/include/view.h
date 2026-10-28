#pragma once

#include <calculator.h>

void view_print_welcome();
void view_print_help();
void view_print_result(double value);
void view_print_status(CalculatorStatus status, const char *error_position);