ROOT := $(abspath ../..)
BUILD_DIR := build

CPPFLAGS := -Iinclude -I../06-07/include
LDFLAGS := -L../06-07/build/lib
LDLIBS := -lchat_common

SOURCES := $(wildcard src/*.c)
TEST_SOURCES := $(wildcard tests/*.c) \
                $(filter-out src/main.c,$(SOURCES))

TARGET := $(BUILD_DIR)/bin/udp_chat
TEST_TARGET := $(BUILD_DIR)/test/bin/test_udp_chat

EXTRA_DEPS := ../06-07/build/lib/libchat_common.a
EXTRA_TEST_DEPS := $(EXTRA_DEPS)

include $(ROOT)/mk/common.mk
include $(ROOT)/mk/app.mk

../06-07/build/lib/libchat_common.a:
	$(MAKE) -C ../06-07

.PHONY: test-integration
test-integration: $(TARGET)
	bash ./tests/run_integration.sh $(abspath $(TARGET))
