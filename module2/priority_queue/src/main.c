#include <menu.h>
#include <priority_queue.h>
#include <program_options.h>
#include <simulation.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  ProgramOptions options;
  ProgramOptionsResult result = program_options_parse(argc, argv, &options);

  if (result == PROGRAM_OPTIONS_HELP) {
    program_options_print_usage(argv[0]);
    return 0;
  }

  if (result == PROGRAM_OPTIONS_ERROR) {
    return 1;
  }

  srand(options.seed);

  PriorityQueue queue;
  priority_queue_initialize(&queue);

  if (options.generate_count > 0) {
    simulation_generate(&queue, options.generate_count,
                        options.maximum_priority);
    printf("Сгенерировано %u сообщений (сид %u, максимум приоритета %u).\n",
           options.generate_count, options.seed, options.maximum_priority);
  }

  if (options.extract_all) {
    simulation_extract_all(&queue);
  } else if (options.extract_priority) {
    simulation_extract_priority(&queue, options.extract_priority_value);
  } else if (options.extract_at_least) {
    simulation_extract_at_least(&queue, options.extract_threshold);
  }

  if (options.run_menu) {
    menu_run(&queue);
  }

  priority_queue_clear(&queue);
  return 0;
}