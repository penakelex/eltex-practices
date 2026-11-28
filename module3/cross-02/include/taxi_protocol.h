#pragma once

#include <stdint.h>
#include <sys/types.h>

typedef enum : int32_t {
  TAXI_COMMAND_TASK,
  TAXI_COMMAND_STATUS,
  TAXI_COMMAND_SHUTDOWN
} TaxiCommandType;

typedef struct {
  TaxiCommandType type;
  uint32_t seconds;
} TaxiCommandMessage;

typedef enum : int32_t {
  TAXI_EVENT_TASK_ACCEPTED,
  TAXI_EVENT_STATUS_AVAILABLE,
  TAXI_EVENT_STATUS_BUSY,
  TAXI_EVENT_AVAILABLE
} TaxiEventType;

typedef struct {
  TaxiEventType type;
  pid_t pid;
  uint32_t seconds;
} TaxiEventMessage;

bool taxi_protocol_write_command(const int fd,
                                 const TaxiCommandMessage *message);
bool taxi_protocol_read_command(const int fd, TaxiCommandMessage *out);
bool taxi_protocol_write_event(const int fd, const TaxiEventMessage *message);
bool taxi_protocol_read_event(const int fd, TaxiEventMessage *out);
