#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "protocol.h"

[[maybe_unused]] constexpr size_t FILE_RECEIVER_PATH_CAPACITY = 512;

bool file_transfer_send(const int fd, const char *sender, const char *path);

typedef struct {
  FILE *file;
  char path[FILE_RECEIVER_PATH_CAPACITY];
  uint64_t expected;
  uint64_t received;
} FileReceiver;

void file_receiver_initialize(FileReceiver *receiver);
void file_receiver_free(FileReceiver *receiver);

bool file_receiver_begin(FileReceiver *receiver, const char *file_name,
                         const uint64_t file_size);
bool file_receiver_append(FileReceiver *receiver, const uint8_t *data,
                          const size_t size);
bool file_receiver_finish(FileReceiver *receiver);
