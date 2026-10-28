#pragma once

#include <stdint.h>

void view_show_menu();
void view_show_prompt(const char *prompt);
void view_show_permission(uint32_t bits);
void view_show_error(const char *message);
void view_show_message(const char *message);
void view_show_ls_comparison(const char *path);