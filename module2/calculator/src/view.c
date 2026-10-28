#include <command.h>
#include <stddef.h>
#include <stdio.h>
#include <view.h>

void view_print_welcome() {
  puts("Калькулятор выражений. Введите '?' для справки, 'q' для выхода.");
}

void view_print_help() {
  puts("Доступные команды:");

  size_t count = 0;
  const Command *all = command_all(&count);

  for (size_t index = 0; index < count; ++index) {
    const char *kind = "";

    switch (all[index].kind) {
    case COMMAND_CONSTANT: {
      kind = "константа";
      break;
    }
    case COMMAND_UNARY: {
      kind = "функция";
      break;
    }
    case COMMAND_BINARY: {
      kind = "операция";
      break;
    }
    }

    printf("  %-8s %s\n", all[index].name, kind);
  }

  puts("");
  puts("Пример: 2 + 3 * sin(pi / 2)");
  puts("'q' — выход");
}

void view_print_result(const double value) {
  printf("%.10g\n", value);
}

void view_print_status(const CalculatorStatus status,
                       const char *error_position) {
  switch (status) {
  case CALCULATOR_OK: {
    return;
  }
  case CALCULATOR_ERROR_PARSE: {
    if (error_position != nullptr && *error_position != '\0') {
      printf("Ошибка разбора рядом с: '%.16s'\n", error_position);
    } else {
      puts("Ошибка разбора выражения.");
    }

    return;
  }
  case CALCULATOR_ERROR_MEMORY: {
    puts("Ошибка выделения памяти.");
    return;
  }
  case CALCULATOR_ERROR_DOMAIN: {
    puts("Ошибка: недопустимая операция (деление на ноль или выход за область "
         "определения).");
    return;
  }
  }
}