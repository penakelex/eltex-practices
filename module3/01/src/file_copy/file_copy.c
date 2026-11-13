#define _POSIX_C_SOURCE 202405L

#include "file_copy.h"
#include "file_copy_internal.h"

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

bool file_copy_run(const FileCopyOptions *options) {
  if (options == nullptr) {
    return false;
  }

  if (options->use_named_pipe && options->fifo_base_name == nullptr) {
    return false;
  }

  FileCopyChannel channel;
  file_copy_channel_initialize(&channel);

  bool initialized;

  if (options->use_named_pipe) {
    initialized =
        file_copy_channel_initialize_named(&channel, options->fifo_base_name);
  } else {
    initialized = file_copy_channel_initialize_unnamed(&channel);
  }

  if (!initialized) {
    file_copy_channel_free(&channel);
    return false;
  }

  signal(SIGPIPE, SIG_IGN);

  const auto child_pid = fork();

  if (child_pid < 0) {
    file_copy_channel_free(&channel);
    return false;
  }

  if (child_pid == 0) {
    const auto child_result = file_copy_child_run(&channel);
    file_copy_channel_free(&channel);
    exit(child_result ? EXIT_SUCCESS : EXIT_FAILURE);
  }

  const auto parent_result = file_copy_parent_run(&channel, options);

  int child_status = 0;

  while (waitpid(child_pid, &child_status, 0) < 0) {
    if (errno != EINTR) {
      break;
    }
  }

  file_copy_channel_free(&channel);

  if (!parent_result) {
    return false;
  }

  if (!WIFEXITED(child_status) || WEXITSTATUS(child_status) != 0) {
    return false;
  }

  return true;
}