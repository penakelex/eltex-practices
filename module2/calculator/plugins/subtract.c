#include <plugin.h>

static double command_subtract(double left, double right) {
  return left - right;
}

PLUGIN_BINARY("-", 1, false, command_subtract)