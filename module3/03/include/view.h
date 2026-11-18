#pragma once

void view_usage(const char *program_name);

void view_invalid_name(const char *queue_name);

void view_connected(const char *name);

void view_no_owner(const char *queue_name);

void view_peer_left(const char *name);

void view_owner_left();

void view_message(const char *from, const char *text);
