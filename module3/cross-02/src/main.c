#include "taxi_controller.h"

int main() {
  if (!taxi_controller_run()) {
    return 1;
  }

  return 0;
}
