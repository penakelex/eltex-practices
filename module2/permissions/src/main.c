#include "input_utils.h"
#include "permissions.h"
#include "view.h"

#include <string.h>

static void handle_input() {
  char buffer[INPUT_LINE_BUFFER_SIZE];

  if (!prompt_for_input("Введите права (например, 755 или rwxr-xr-x): ", buffer,
                        sizeof(buffer))) {
    return;
  }

  const PermissionResult result = permission_from_input(buffer);

  if (result.status != PERMISSION_STATUS_OK) {
    view_show_error("не удалось разобрать ввод");
    return;
  }

  view_show_permission(result.bits);
}

static void handle_file() {
  char buffer[INPUT_LINE_BUFFER_SIZE];

  if (!prompt_for_input("Введите имя файла: ", buffer, sizeof(buffer))) {
    return;
  }

  if (buffer[0] == '\0') {
    view_show_error("пустое имя файла");
    return;
  }

  const PermissionResult result = permission_from_file(buffer);

  if (result.status != PERMISSION_STATUS_OK) {
    view_show_error("не удалось получить информацию о файле (stat)");
    return;
  }

  view_show_permission(result.bits);
  view_show_ls_comparison(buffer);
}

static void handle_modify() {
  char current_buffer[INPUT_LINE_BUFFER_SIZE];
  char command_buffer[INPUT_LINE_BUFFER_SIZE];

  if (!prompt_for_input("Текущие права (например, 755 или rwxr-xr-x): ",
                        current_buffer, sizeof(current_buffer))) {
    return;
  }

  const PermissionResult current = permission_from_input(current_buffer);

  if (current.status != PERMISSION_STATUS_OK) {
    view_show_error("не удалось разобрать текущие права");
    return;
  }

  if (!prompt_for_input("Команда модификации (например, u+x,g-w): ",
                        command_buffer, sizeof(command_buffer))) {
    return;
  }

  const PermissionResult modified =
      permission_from_modify(current.bits, command_buffer);

  if (modified.status != PERMISSION_STATUS_OK) {
    view_show_error("не удалось применить команду модификации");
    return;
  }

  view_show_message("Исходные права:");
  view_show_permission(current.bits);
  view_show_message("Новые права:");
  view_show_permission(modified.bits);
}

int main() {
  view_show_menu();

  char buffer[INPUT_LINE_BUFFER_SIZE];

  while (true) {
    if (!prompt_for_input("> ", buffer, sizeof(buffer))) {
      break;
    }

    if (strcmp(buffer, "q") == 0) {
      break;
    }

    if (strcmp(buffer, "?") == 0) {
      view_show_menu();
      continue;
    }

    if (strcmp(buffer, "1") == 0) {
      handle_input();
    } else if (strcmp(buffer, "2") == 0) {
      handle_file();
    } else if (strcmp(buffer, "3") == 0) {
      handle_modify();
    } else if (buffer[0] != '\0') {
      view_show_error("неизвестная команда");
    }
  }

  view_show_message("До свидания!");
  return 0;
}