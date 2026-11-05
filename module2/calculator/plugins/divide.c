#include <math.h>
#include <plugin.h>

static double command_divide(double left, double right) {
  if (right == 0.0) {
    return NAN;
  }

  return left / right;
}

PLUGIN_BINARY("/", 2, false, command_divide)