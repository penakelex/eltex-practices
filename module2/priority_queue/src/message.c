#include <message.h>
#include <stdio.h>

void message_print(const Message *message) {
  printf("id=%u priority=%u payload=\"%s\"\n", message->id, message->priority,
         message->payload);
}