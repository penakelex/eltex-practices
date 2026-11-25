#pragma once

void view_usage(const char *program_name, const char *hint);
void view_error(const char *message);
void view_connected(const char *name);
void view_joined(const char *name);
void view_left(const char *name);
void view_message(const char *sender, const char *text);
void view_file_incoming(const char *sender, const char *file_name);
void view_file_saved(const char *path);
