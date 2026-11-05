#include <plugin.h>

static double command_multiply(double left, double right) {
  return left * right;
}

PLUGIN_BINARY("*", 2, false, command_multiply)