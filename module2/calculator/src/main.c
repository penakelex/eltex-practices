#include <calculator.h>
#include <input_utils.h>
#include <stdbool.h>
#include <string.h>
#include <view.h>

static bool is_quit(const char *line) {
  return strcmp(line, "q") == 0 || strcmp(line, "quit") == 0;
}

static bool is_help(const char *line) {
  return strcmp(line, "?") == 0 || strcmp(line, "help") == 0;
}

int main() {
  view_print_welcome();

  char line[INPUT_LINE_BUFFER_SIZE];

  while (true) {
    prompt_for_input("> ", line, sizeof line);

    if (is_quit(line)) {
      break;
    }

    if (is_help(line)) {
      view_print_help();
      continue;
    }

    if (line[0] == '\0') {
      continue;
    }

    double result = 0.0;
    const char *error_position = nullptr;
    const CalculatorStatus status =
        calculator_evaluate(line, &result, &error_position);

    if (status == CALCULATOR_OK) {
      view_print_result(result);
    } else {
      view_print_status(status, error_position);
    }
  }

  return 0;
}