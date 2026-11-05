#include "command.h"
#include <calculator.h>
#include <input_utils.h>
#include <loader.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string_utils.h>
#include <view.h>

static bool is_quit(const char *line) {
  return strcmp(line, "q") == 0 || strcmp(line, "quit") == 0;
}

static bool is_help(const char *line) {
  return strcmp(line, "?") == 0 || strcmp(line, "help") == 0;
}

static bool is_reload(const char *line) {
  return strcmp(line, "reload") == 0;
}

static void print_plugin_summary(void) {
  size_t count = 0;
  command_all(&count);
  printf("Плагины перезагружены. Доступно команд: %zu. "
         "Введите '?' для просмотра списка.\n",
         count);
}

static char *plugins_directory_from_binary(const char *argv0) {
  if (argv0 == nullptr) {
    return duplicate_string("build/lib/plugins");
  }

  const char *slash = strrchr(argv0, '/');

  if (slash == nullptr) {
    return duplicate_string("build/lib/plugins");
  }

  const size_t prefix_length = (size_t)(slash - argv0);
  const char *suffix = "/../lib/plugins";
  const size_t suffix_length = strlen(suffix);
  char *path = malloc(prefix_length + suffix_length + 1);

  if (path == nullptr) {
    return nullptr;
  }

  if (prefix_length > 0) {
    memcpy(path, argv0, prefix_length);
  }

  memcpy(path + prefix_length, suffix, suffix_length + 1);
  return path;
}

static char *choose_plugins_directory(int argc, char **argv) {
  if (argc > 1) {
    return duplicate_string(argv[1]);
  }

  const char *from_environment = getenv("CALCULATOR_PLUGINS_DIR");

  if (from_environment != nullptr && from_environment[0] != '\0') {
    return duplicate_string(from_environment);
  }

  return plugins_directory_from_binary(argv[0]);
}

int main(int argc, char **argv) {
  char *plugins_directory = choose_plugins_directory(argc, argv);

  if (plugins_directory == nullptr) {
    fprintf(stderr, "Не удалось определить каталог плагинов\n");
    return 1;
  }

  CommandLoader loader = {0};

  if (!command_loader_load_directory(&loader, plugins_directory)) {
    fprintf(stderr, "Не удалось открыть каталог плагинов: %s\n",
            plugins_directory);
    free(plugins_directory);
    return 1;
  }

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

    if (is_reload(line)) {
      if (command_loader_reload(&loader, plugins_directory)) {
        print_plugin_summary();
      } else {
        puts("Не удалось перезагрузить плагины. Проверьте каталог и "
             "выполните 'reload' ещё раз.");
      }
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

  command_reset();
  command_loader_unload(&loader);
  free(plugins_directory);
  return 0;
}