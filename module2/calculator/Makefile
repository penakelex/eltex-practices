ROOT := $(abspath ../..)
BUILD_DIR := build

CPPFLAGS := -Iinclude -I../common/include
LDFLAGS  := -L../common/build/lib
LDLIBS   := -lcommon -lm -ldl

PLUGIN_DIR := $(BUILD_DIR)/lib/plugins
PLUGIN_SOURCES := $(wildcard plugins/*.c)
PLUGIN_LIBS := $(patsubst plugins/%.c,$(PLUGIN_DIR)/lib%.so,$(PLUGIN_SOURCES))

SOURCES := $(wildcard src/*.c)
TEST_SOURCES := $(wildcard tests/*.c) $(filter-out src/main.c,$(SOURCES))

TARGET := $(BUILD_DIR)/bin/calculator
TEST_TARGET := $(BUILD_DIR)/test/bin/test_calculator

EXTRA_DEPS := $(PLUGIN_LIBS) ../common/build/lib/libcommon.a
EXTRA_TEST_DEPS := $(EXTRA_DEPS)

include $(ROOT)/mk/common.mk
CFLAGS += -DUNITY_INCLUDE_DOUBLE
include $(ROOT)/mk/app.mk

$(PLUGIN_DIR)/lib%.so: plugins/%.c plugins/plugin.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -Iplugins -fPIC -shared -o $@ $< -lm

../common/build/lib/libcommon.a:
	$(MAKE) -C ../common

plugins: $(PLUGIN_LIBS)
.PHONY: plugins