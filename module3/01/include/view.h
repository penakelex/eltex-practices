#pragma once

void file_copy_view_usage(const char *program_name);

void file_copy_view_open_error(const char *file_name, int error_code);

void file_copy_view_stat_error(const char *file_name, int error_code);

void file_copy_view_not_regular_file(const char *file_name);

void file_copy_view_name_too_long(const char *file_name);

void file_copy_view_create_error(const char *file_name, int error_code);