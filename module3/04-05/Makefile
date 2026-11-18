ROOT := $(abspath ../..)
BUILD_DIR := build
CPPFLAGS := -Iinclude

LIBRARY := $(BUILD_DIR)/lib/libpc_common.a
SOURCES := $(wildcard src/*.c)
TEST_SOURCES := $(wildcard tests/*.c) $(SOURCES)
TEST_TARGET := $(BUILD_DIR)/test/bin/test_pc_common

all: $(LIBRARY)

include $(ROOT)/mk/common.mk
include $(ROOT)/mk/lib.mk

$(LIBRARY): $(OBJECTS)
	mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $^
