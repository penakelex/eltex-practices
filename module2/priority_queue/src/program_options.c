#include <input_utils.h>
#include <message.h>
#include <program_options.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void program_options_defaults(ProgramOptions *options) {
  options->seed = 1;
  options->generate_count = 0;
  options->maximum_priority = (unsigned int)(PRIORITY_LEVELS - 1);
  options->extract_all = false;
  options->extract_priority = false;
  options->extract_at_least = false;
  options->extract_priority_value = 0;
  options->extract_threshold = 0;
  options->run_menu = true;
}

void program_options_print_usage(const char *program) {
  printf("Использование: %s [опции]\n", program);
  puts("Без аргументов запускает интерактивное меню.");
  puts("Опции:");
  puts("  --help                  справка");
  puts("  --seed N                сид генератора (по умолчанию 1)");
  puts("  --generate N            сгенерировать N сообщений перед извлечением");
  puts("  --max-priority M        максимум приоритета при генерации (по "
       "умолчанию 255)");
  puts("  --extract-all           извлечь все сообщения по порядку");
  puts("  --extract-priority P    извлечь все с приоритетом P");
  puts("  --extract-at-least T    извлечь все с приоритетом >= T");
}

static bool next_uint_argument(int argc, char **argv, int *position,
                               uint32_t *out) {
  if (*position + 1 >= argc) {
    return false;
  }

  if (!parse_uint32(argv[*position + 1], out)) {
    return false;
  }

  *position += 1;
  return true;
}

static ProgramOptionsResult parse_uint_option(int argc, char **argv,
                                              int *position, uint32_t *value) {
  if (!next_uint_argument(argc, argv, position, value)) {
    fprintf(stderr, "Ожидается число после %s\n", argv[*position]);
    return PROGRAM_OPTIONS_ERROR;
  }

  return PROGRAM_OPTIONS_OK;
}

ProgramOptionsResult program_options_parse(int argc, char **argv,
                                           ProgramOptions *options) {
  program_options_defaults(options);

  for (int index = 1; index < argc; ++index) {
    if (strcmp(argv[index], "--help") == 0 || strcmp(argv[index], "-h") == 0) {
      return PROGRAM_OPTIONS_HELP;
    }

    if (strcmp(argv[index], "--seed") == 0) {
      uint32_t value;

      if (parse_uint_option(argc, argv, &index, &value) != PROGRAM_OPTIONS_OK) {
        return PROGRAM_OPTIONS_ERROR;
      }

      options->seed = value;
    } else if (strcmp(argv[index], "--generate") == 0) {
      uint32_t value;

      if (parse_uint_option(argc, argv, &index, &value) != PROGRAM_OPTIONS_OK) {
        return PROGRAM_OPTIONS_ERROR;
      }

      options->generate_count = value;
    } else if (strcmp(argv[index], "--max-priority") == 0) {
      uint32_t value;

      if (parse_uint_option(argc, argv, &index, &value) != PROGRAM_OPTIONS_OK) {
        return PROGRAM_OPTIONS_ERROR;
      }

      if (value >= PRIORITY_LEVELS) {
        value = (uint32_t)(PRIORITY_LEVELS - 1);
      }

      options->maximum_priority = value;
    } else if (strcmp(argv[index], "--extract-all") == 0) {
      options->extract_all = true;
      options->run_menu = false;
    } else if (strcmp(argv[index], "--extract-priority") == 0) {
      uint32_t value;

      if (parse_uint_option(argc, argv, &index, &value) != PROGRAM_OPTIONS_OK) {
        return PROGRAM_OPTIONS_ERROR;
      }

      options->extract_priority = true;
      options->extract_priority_value = value;
      options->run_menu = false;
    } else if (strcmp(argv[index], "--extract-at-least") == 0) {
      uint32_t value;

      if (parse_uint_option(argc, argv, &index, &value) != PROGRAM_OPTIONS_OK) {
        return PROGRAM_OPTIONS_ERROR;
      }

      options->extract_at_least = true;
      options->extract_threshold = value;
      options->run_menu = false;
    } else {
      fprintf(stderr, "Неизвестный аргумент: %s\n", argv[index]);
      return PROGRAM_OPTIONS_ERROR;
    }
  }

  return PROGRAM_OPTIONS_OK;
}