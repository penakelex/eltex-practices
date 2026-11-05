#include <plugin.h>

static double command_negate(double value) {
  return -value;
}

PLUGIN_UNARY("neg", 4, command_negate)