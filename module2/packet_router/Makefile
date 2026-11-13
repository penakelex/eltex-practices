ROOT := $(abspath ../..)
BUILD_DIR := build

CPPFLAGS := -Iinclude -I../common/include
LDFLAGS  := -L../common/build/lib
LDLIBS   := -lcommon

SOURCES := $(wildcard src/*.c)
TEST_SOURCES := $(wildcard tests/*.c) \
                $(filter-out src/main.c,$(SOURCES))

TARGET := $(BUILD_DIR)/bin/packet_router
TEST_TARGET := $(BUILD_DIR)/test/bin/test_packet_router

EXTRA_DEPS := ../common/build/lib/libcommon.a
EXTRA_TEST_DEPS := $(EXTRA_DEPS)

include $(ROOT)/mk/common.mk
include $(ROOT)/mk/app.mk

../common/build/lib/libcommon.a:
	$(MAKE) -C ../common