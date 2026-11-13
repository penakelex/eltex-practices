#pragma once

#include <stddef.h>
#include <sys/types.h>

[[maybe_unused]] constexpr size_t QUEUE_MESSAGE_MAX_TEXT = 4096;
[[maybe_unused]] constexpr long QUEUE_MESSAGE_TYPE_CONTROL = 1;

typedef struct {
  long type;
  char text[QUEUE_MESSAGE_MAX_TEXT];
} QueueMessage;

int queue_create();
int queue_open();
void queue_remove(int msqid);

bool queue_send(int msqid, long type, const char *text);
ssize_t queue_receive(int msqid, long type, QueueMessage *out);