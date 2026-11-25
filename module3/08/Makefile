ROOT := $(abspath ../..)
BUILD_DIR := build

CPPFLAGS := -Iinclude

SOURCES := $(wildcard src/*.c)
TEST_SOURCES := $(wildcard tests/*.c) $(filter-out src/main.c,$(SOURCES))

TARGET := $(BUILD_DIR)/bin/udp_sniffer
TEST_TARGET := $(BUILD_DIR)/test/bin/test_udp_sniffer

include $(ROOT)/mk/common.mk
include $(ROOT)/mk/app.mk
