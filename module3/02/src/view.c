#define _POSIX_C_SOURCE 202405L

#include "view.h"

#include <stdio.h>

void view_usage(const char *program_name) {
  fprintf(stderr, "Usage: %s -b | -p <topic> | -s <topic> [<topic> ...]\n",
          program_name);
}

void view_queue_exists() {
  fprintf(stderr, "Broker already running on this queue\n");
}

void view_no_broker() {
  fprintf(stderr, "Broker is not running\n");
}

void view_queue_gone() {
  fprintf(stderr, "Queue is gone, exiting\n");
}

void view_broker_started() {
  fprintf(stderr, "Broker started\n");
}

void view_broker_stopped() {
  fprintf(stderr, "Broker stopped\n");
}

void view_subscriber_message(const char *topic, const char *payload) {
  printf("[%s] %s\n", topic, payload);
  fflush(stdout);
}