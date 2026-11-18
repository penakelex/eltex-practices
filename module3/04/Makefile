ROOT := $(abspath ../..)
BUILD_DIR := build

CPPFLAGS := -Iinclude -I../04-05/include
LDFLAGS := -L../04-05/build/lib
LDLIBS := -lpc_common

SOURCES := $(wildcard src/*.c)
TEST_SOURCES := $(wildcard tests/*.c) \
                $(filter-out src/main.c,$(SOURCES))

TARGET := $(BUILD_DIR)/bin/producer_consumer
TEST_TARGET := $(BUILD_DIR)/test/bin/test_producer_consumer

EXTRA_DEPS := ../04-05/build/lib/libpc_common.a
EXTRA_TEST_DEPS := $(EXTRA_DEPS)

include $(ROOT)/mk/common.mk
include $(ROOT)/mk/app.mk

../04-05/build/lib/libpc_common.a:
	$(MAKE) -C ../04-05

.PHONY: test-integration
test-integration: $(TARGET)
	bash ./tests/run_integration.sh $(abspath $(TARGET))
