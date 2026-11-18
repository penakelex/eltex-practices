#pragma once

#include "file_copy.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
  int to_child_read_fd;
  int to_child_write_fd;
  int to_parent_read_fd;
  int to_parent_write_fd;
  char *fifo_to_child_name;
  char *fifo_to_parent_name;
} FileCopyChannel;

void file_copy_channel_initialize(FileCopyChannel *self);

void file_copy_channel_free(FileCopyChannel *self);

void file_copy_channel_close_fds(FileCopyChannel *self);

bool file_copy_channel_initialize_unnamed(FileCopyChannel *self);

bool file_copy_channel_initialize_named(FileCopyChannel *self,
                                        const char *fifo_base_name);

bool file_copy_read_full(const int fd, void *buffer, const size_t size);

bool file_copy_write_full(const int fd, const void *buffer, const size_t size);

bool file_copy_child_run(FileCopyChannel *channel);

bool file_copy_parent_run(FileCopyChannel *channel,
                          const FileCopyOptions *options);

char *file_copy_join_strings(const char *first, const char *second);