#pragma once

typedef struct {
  unsigned int seed;
  unsigned int generate_count;
  unsigned int maximum_priority;
  bool extract_all;
  bool extract_priority;
  bool extract_at_least;
  unsigned int extract_priority_value;
  unsigned int extract_threshold;
  bool run_menu;
} ProgramOptions;

typedef enum {
  PROGRAM_OPTIONS_OK,
  PROGRAM_OPTIONS_HELP,
  PROGRAM_OPTIONS_ERROR,
} ProgramOptionsResult;

void program_options_defaults(ProgramOptions *options);
void program_options_print_usage(const char *program);
ProgramOptionsResult program_options_parse(int argc, char **argv,
                                           ProgramOptions *options);