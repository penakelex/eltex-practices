#define _POSIX_C_SOURCE 202405L

#include "file_copy.h"
#include "file_copy_internal.h"
#include "view.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

bool file_copy_child_run(FileCopyChannel *channel) {
  if (channel->fifo_to_child_name != nullptr) {
    channel->to_child_read_fd = open(channel->fifo_to_child_name, O_RDONLY);

    if (channel->to_child_read_fd < 0) {
      return false;
    }

    channel->to_parent_write_fd = open(channel->fifo_to_parent_name, O_WRONLY);

    if (channel->to_parent_write_fd < 0) {
      return false;
    }
  } else {
    close(channel->to_child_write_fd);
    close(channel->to_parent_read_fd);
    channel->to_child_write_fd = -1;
    channel->to_parent_read_fd = -1;
  }

  const char ready = 'R';

  if (!file_copy_write_full(channel->to_parent_write_fd, &ready, 1)) {
    return false;
  }

  char name[FILE_COPY_MAX_NAME_LENGTH + 1];
  char buffer[FILE_COPY_BLOCK_SIZE];

  while (true) {
    uint32_t name_length = 0;
    uint64_t size = 0;

    if (!file_copy_read_header(channel->to_child_read_fd, name, sizeof(name),
                               &name_length, &size)) {
      return false;
    }

    if (name_length == 0) {
      break;
    }

    auto copy_name = file_copy_join_strings(name, ".copy");

    if (copy_name == nullptr) {
      return false;
    }

    auto output_fd = open(copy_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (output_fd < 0) {
      const auto error_code = errno;
      file_copy_view_create_error(name, error_code);
    }

    free(copy_name);

    uint64_t remaining = size;

    while (remaining > 0) {
      const auto block_size = remaining < FILE_COPY_BLOCK_SIZE
                                  ? (size_t)remaining
                                  : FILE_COPY_BLOCK_SIZE;

      if (!file_copy_read_full(channel->to_child_read_fd, buffer, block_size)) {
        if (output_fd >= 0) {
          close(output_fd);
        }

        return false;
      }

      if (output_fd >= 0 &&
          !file_copy_write_full(output_fd, buffer, block_size)) {
        close(output_fd);
        output_fd = -1;
      }

      remaining -= block_size;
    }

    if (output_fd >= 0) {
      close(output_fd);
    }
  }

  file_copy_channel_close_fds(channel);

  return true;
}