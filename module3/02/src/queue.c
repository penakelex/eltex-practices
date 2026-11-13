#define _POSIX_C_SOURCE 202405L

#include "queue.h"

#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

static key_t queue_key() {
  return ftok("/tmp", 'M');
}

int queue_create() {
  const auto key = queue_key();

  if (key == (key_t)-1) {
    return -1;
  }

  return msgget(key, IPC_CREAT | IPC_EXCL | 0600);
}

int queue_open() {
  const auto key = queue_key();

  if (key == (key_t)-1) {
    return -1;
  }

  return msgget(key, 0);
}

void queue_remove(const int msqid) {
  msgctl(msqid, IPC_RMID, nullptr);
}

bool queue_send(const int msqid, const long type, const char *text) {
  if (text == nullptr) {
    return false;
  }

  QueueMessage message;
  message.type = type;

  const auto length = strlen(text);

  if (length > sizeof(message.text) - 1) {
    return false;
  }

  memcpy(message.text, text, length);

  return msgsnd(msqid, &message, length, 0) == 0;
}

ssize_t queue_receive(const int msqid, const long type, QueueMessage *out) {
  if (out == nullptr) {
    return -1;
  }

  const auto received = msgrcv(msqid, out, sizeof(out->text) - 1, type, 0);

  if (received < 0) {
    return -1;
  }

  out->text[received] = '\0';

  return received;
}