ROOT := $(abspath ../..)
BUILD_DIR := build

CPPFLAGS := -Iinclude -I../06-07/include
LDFLAGS := -L../06-07/build/lib
LDLIBS := -lchat_common

SOURCES := $(filter-out src/server_main.c,$(wildcard src/*.c))
TEST_SOURCES := $(wildcard tests/*.c) \
                $(filter-out src/server_main.c src/client_main.c, \
                             $(wildcard src/*.c))

TARGET := $(BUILD_DIR)/bin/chat_client
TEST_TARGET := $(BUILD_DIR)/test/bin/test_chat

EXTRA_DEPS := ../06-07/build/lib/libchat_common.a
EXTRA_TEST_DEPS := $(EXTRA_DEPS)

include $(ROOT)/mk/common.mk
include $(ROOT)/mk/app.mk

SERVER_TARGET := $(BUILD_DIR)/bin/chat_server
SERVER_SOURCES := $(filter-out src/client_main.c,$(wildcard src/*.c))
SERVER_OBJECTS = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SERVER_SOURCES))

all: $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_OBJECTS) $(EXTRA_DEPS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJECTS) $(LDFLAGS) $(LDLIBS)

.PHONY: run-server
run-server: $(SERVER_TARGET)
	./$(SERVER_TARGET) $(ARGS)

../06-07/build/lib/libchat_common.a:
	$(MAKE) -C ../06-07

.PHONY: test-integration
test-integration: $(TARGET) $(SERVER_TARGET)
	bash ./tests/run_integration.sh $(abspath $(SERVER_TARGET)) \
	  $(abspath $(TARGET))
