ROOT := $(abspath ../..)
BUILD_DIR := build
CPPFLAGS := -Iinclude

SOURCES := $(wildcard src/*.c)
TEST_SOURCES := $(wildcard tests/*.c) \
                $(filter-out src/main.c,$(SOURCES))

TARGET := $(BUILD_DIR)/bin/broker_chat
TEST_TARGET := $(BUILD_DIR)/test/bin/test_broker_chat

include $(ROOT)/mk/common.mk
include $(ROOT)/mk/app.mk

.PHONY: test-integration
test-integration: $(TARGET)
	bash ./tests/run_integration.sh $(abspath $(TARGET))