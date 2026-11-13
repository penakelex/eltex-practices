#pragma once

#include <stddef.h>
#include <stdint.h>

[[maybe_unused]] constexpr size_t FILE_COPY_BLOCK_SIZE = 4096;
[[maybe_unused]] constexpr size_t FILE_COPY_MAX_NAME_LENGTH = 4096;

typedef struct {
  bool use_named_pipe;
  const char *fifo_base_name;
  int file_count;
  char *const *file_names;
} FileCopyOptions;

bool file_copy_run(const FileCopyOptions *options);

bool file_copy_write_header(int fd, const char *name, uint64_t size);

bool file_copy_read_header(int fd, char *name_out, size_t name_capacity,
                           uint32_t *name_length_out, uint64_t *size_out);