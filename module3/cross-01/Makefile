ROOT := $(abspath ../..)
BUILD_DIR := build

CPPFLAGS := -Iinclude

SOURCES := $(wildcard src/*.c)
TEST_SOURCES := $(wildcard tests/*.c) $(filter-out src/main.c,$(SOURCES))

TARGET := $(BUILD_DIR)/bin/echo_raw
TEST_TARGET := $(BUILD_DIR)/test/bin/test_echo_raw

include $(ROOT)/mk/common.mk
include $(ROOT)/mk/app.mk

.PHONY: run-server run-client

run-server: $(TARGET)
	sudo ./$(TARGET) -s $(ARGS)

run-client: $(TARGET)
	sudo ./$(TARGET) -c 127.0.0.1 $(ARGS)
