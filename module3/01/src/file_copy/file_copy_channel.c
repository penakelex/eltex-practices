#define _POSIX_C_SOURCE 202405L

#include "file_copy_internal.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

char *file_copy_join_strings(const char *first, const char *second) {
  const auto first_length = strlen(first);
  const auto second_length = strlen(second);
  auto result = (char *)malloc(first_length + second_length + 1);

  if (result == nullptr) {
    return nullptr;
  }

  memcpy(result, first, first_length);
  memcpy(result + first_length, second, second_length + 1);

  return result;
}

void file_copy_channel_initialize(FileCopyChannel *self) {
  self->to_child_read_fd = -1;
  self->to_child_write_fd = -1;
  self->to_parent_read_fd = -1;
  self->to_parent_write_fd = -1;
  self->fifo_to_child_name = nullptr;
  self->fifo_to_parent_name = nullptr;
}

void file_copy_channel_close_fds(FileCopyChannel *self) {
  if (self->to_child_read_fd >= 0) {
    close(self->to_child_read_fd);
    self->to_child_read_fd = -1;
  }

  if (self->to_child_write_fd >= 0) {
    close(self->to_child_write_fd);
    self->to_child_write_fd = -1;
  }

  if (self->to_parent_read_fd >= 0) {
    close(self->to_parent_read_fd);
    self->to_parent_read_fd = -1;
  }

  if (self->to_parent_write_fd >= 0) {
    close(self->to_parent_write_fd);
    self->to_parent_write_fd = -1;
  }
}

void file_copy_channel_free(FileCopyChannel *self) {
  file_copy_channel_close_fds(self);

  if (self->fifo_to_child_name != nullptr) {
    unlink(self->fifo_to_child_name);
    free(self->fifo_to_child_name);
    self->fifo_to_child_name = nullptr;
  }

  if (self->fifo_to_parent_name != nullptr) {
    unlink(self->fifo_to_parent_name);
    free(self->fifo_to_parent_name);
    self->fifo_to_parent_name = nullptr;
  }
}

bool file_copy_channel_initialize_unnamed(FileCopyChannel *self) {
  int to_child[2];
  int to_parent[2];

  if (pipe(to_child) < 0) {
    return false;
  }

  if (pipe(to_parent) < 0) {
    close(to_child[0]);
    close(to_child[1]);
    return false;
  }

  self->to_child_read_fd = to_child[0];
  self->to_child_write_fd = to_child[1];
  self->to_parent_read_fd = to_parent[0];
  self->to_parent_write_fd = to_parent[1];

  return true;
}

bool file_copy_channel_initialize_named(FileCopyChannel *self,
                                        const char *fifo_base_name) {
  self->fifo_to_child_name = file_copy_join_strings(fifo_base_name, ".p2c");

  if (self->fifo_to_child_name == nullptr) {
    return false;
  }

  self->fifo_to_parent_name = file_copy_join_strings(fifo_base_name, ".c2p");

  if (self->fifo_to_parent_name == nullptr) {
    return false;
  }

  unlink(self->fifo_to_child_name);
  unlink(self->fifo_to_parent_name);

  if (mkfifo(self->fifo_to_child_name, 0600) < 0) {
    return false;
  }

  if (mkfifo(self->fifo_to_parent_name, 0600) < 0) {
    return false;
  }

  return true;
}