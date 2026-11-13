#define _POSIX_C_SOURCE 202405L

#include "file_copy.h"
#include "file_copy_internal.h"
#include "view.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

bool file_copy_parent_run(FileCopyChannel *channel,
                          const FileCopyOptions *options) {
  if (channel->fifo_to_child_name != nullptr) {
    channel->to_child_write_fd = open(channel->fifo_to_child_name, O_WRONLY);

    if (channel->to_child_write_fd < 0) {
      return false;
    }

    channel->to_parent_read_fd = open(channel->fifo_to_parent_name, O_RDONLY);

    if (channel->to_parent_read_fd < 0) {
      return false;
    }
  } else {
    close(channel->to_child_read_fd);
    close(channel->to_parent_write_fd);
    channel->to_child_read_fd = -1;
    channel->to_parent_write_fd = -1;
  }

  char ready = 0;

  if (!file_copy_read_full(channel->to_parent_read_fd, &ready, 1)) {
    return false;
  }

  char buffer[FILE_COPY_BLOCK_SIZE];

  for (int i = 0; i < options->file_count; ++i) {
    const char *file_name = options->file_names[i];
    const auto input_fd = open(file_name, O_RDONLY);

    if (input_fd < 0) {
      const auto error_code = errno;
      file_copy_view_open_error(file_name, error_code);
      continue;
    }

    struct stat file_status;

    if (fstat(input_fd, &file_status) < 0) {
      const auto error_code = errno;
      file_copy_view_stat_error(file_name, error_code);
      close(input_fd);
      continue;
    }

    if (!S_ISREG(file_status.st_mode)) {
      file_copy_view_not_regular_file(file_name);
      close(input_fd);
      continue;
    }

    const auto name_length = strlen(file_name);

    if (name_length > FILE_COPY_MAX_NAME_LENGTH) {
      file_copy_view_name_too_long(file_name);
      close(input_fd);
      continue;
    }

    const auto size = (uint64_t)file_status.st_size;

    if (!file_copy_write_header(channel->to_child_write_fd, file_name, size)) {
      close(input_fd);
      return false;
    }

    uint64_t remaining = size;

    while (remaining > 0) {
      const auto block_size = remaining < FILE_COPY_BLOCK_SIZE
                                  ? (size_t)remaining
                                  : FILE_COPY_BLOCK_SIZE;
      const auto read_result = read(input_fd, buffer, block_size);

      if (read_result < 0) {
        if (errno == EINTR) {
          continue;
        }

        close(input_fd);
        return false;
      }

      if (read_result == 0) {
        break;
      }

      if (!file_copy_write_full(channel->to_child_write_fd, buffer,
                                (size_t)read_result)) {
        close(input_fd);
        return false;
      }

      remaining -= (uint64_t)read_result;
    }

    close(input_fd);
  }

  const auto sent =
      file_copy_write_header(channel->to_child_write_fd, nullptr, 0);

  file_copy_channel_close_fds(channel);

  return sent;
}