#include "unity.h"
#include <command.h>
#include <loader.h>
#include <stdio.h>

void test_ast_run();
void test_command_run();
void test_parser_run();
void test_calculator_run();

static CommandLoader loader = {0};

void setUp() {
}

void tearDown() {
}

int main() {
  if (!command_loader_load_directory(&loader, "build/lib/plugins")) {
    fprintf(stderr, "Не удалось загрузить плагины\n");
    return 1;
  }

  UNITY_BEGIN();
  RUN_TEST(test_ast_run);
  RUN_TEST(test_command_run);
  RUN_TEST(test_parser_run);
  RUN_TEST(test_calculator_run);
  const int status = UNITY_END();

  command_reset();
  command_loader_unload(&loader);
  return status;
}