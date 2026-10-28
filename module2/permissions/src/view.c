#define _POSIX_C_SOURCE 202405L

#include "view.h"

#include "permissions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void view_show_menu() {
  printf("\n=== Калькулятор прав доступа ===\n");
  printf("1. Ввести права в буквенном или цифровом обозначении\n");
  printf("2. Получить права из файла (stat)\n");
  printf("3. Модифицировать права (chmod-подобно)\n");
  printf("?. Повторить меню\n");
  printf("q. Выход\n");
}

void view_show_prompt(const char *prompt) {
  printf("%s", prompt);
}

void view_show_permission(uint32_t bits) {
  char *symbolic = permission_format_symbolic(bits);
  char *octal = permission_format_octal(bits);
  char *binary = permission_format_binary(bits);

  printf("  Буквенное: %s\n", symbolic != nullptr ? symbolic : "<error>");
  printf("  Цифровое:  %s\n", octal != nullptr ? octal : "<error>");
  printf("  Битовое:   %s\n", binary != nullptr ? binary : "<error>");

  free(symbolic);
  free(octal);
  free(binary);
}

void view_show_error(const char *message) {
  printf("Ошибка: %s\n", message);
}

void view_show_message(const char *message) {
  printf("%s\n", message);
}

static void shell_escape(const char *input, char *output, size_t output_size) {
  size_t out = 0;

  for (const char *p = input; *p != '\0' && out + 6 < output_size; p++) {
    if (*p == '\'') {
      output[out++] = '\'';
      output[out++] = '\\';
      output[out++] = '\'';
      output[out++] = '\'';
    } else {
      output[out++] = *p;
    }
  }

  output[out] = '\0';
}

void view_show_ls_comparison(const char *path) {
  char escaped[1024];
  shell_escape(path, escaped, sizeof(escaped));

  char command[1200];
  const int written =
      snprintf(command, sizeof(command), "ls -ld -- '%s' 2>/dev/null", escaped);

  if (written < 0 || (size_t)written >= sizeof(command)) {
    return;
  }

  FILE *pipe = popen(command, "r");

  if (pipe == nullptr) {
    return;
  }

  char line[512];
  bool had_output = false;

  while (fgets(line, sizeof(line), pipe) != nullptr) {
    if (!had_output) {
      printf("\nДля сравнения `ls -ld`:\n");
      had_output = true;
    }

    printf("  %s", line);
  }

  pclose(pipe);

  if (!had_output) {
    printf("\n(не удалось выполнить ls -ld для сравнения)\n");
  }
}