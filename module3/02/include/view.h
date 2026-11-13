#pragma once

void view_usage(const char *program_name);

void view_queue_exists();

void view_no_broker();

void view_queue_gone();

void view_broker_started();

void view_broker_stopped();

void view_subscriber_message(const char *topic, const char *payload);