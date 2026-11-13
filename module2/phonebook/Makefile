ROOT := $(abspath ../..)
BUILD_DIR := build

CPPFLAGS := -Iinclude -I../common/include -I../list/include
LDLIBS   := -lcommon

COMMON_LIB   := ../common/build/lib/libcommon.a
LIST_STATIC  := ../list/build/lib/liblist.a
LIST_DYNAMIC := ../list/build/lib/liblist.so

TARGET_DYNAMIC := $(BUILD_DIR)/bin/phonebook
TARGET_STATIC  := $(BUILD_DIR)/bin/phonebook-static
TEST_TARGET_DYNAMIC := $(BUILD_DIR)/test/bin/test_phonebook
TEST_TARGET_STATIC  := $(BUILD_DIR)/test/bin/test_phonebook-static

OBJ_DIR      := $(BUILD_DIR)/obj
TEST_OBJ_DIR := $(BUILD_DIR)/test/obj
SOURCES := $(wildcard src/*.c) $(wildcard src/collections/*.c)
TEST_SOURCES := $(wildcard tests/*.c) \
                $(filter-out src/main.c,$(wildcard src/*.c)) \
                $(wildcard src/collections/*.c)

include $(ROOT)/mk/common.mk

OBJECTS      := $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS         := $(OBJECTS:.o=.d)
TEST_OBJECTS := $(patsubst %.c,$(TEST_OBJ_DIR)/%.o,$(TEST_SOURCES))
TEST_DEPS    := $(TEST_OBJECTS:.o=.d)

all: $(TARGET_DYNAMIC) $(TARGET_STATIC)

$(OBJ_DIR)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

$(TEST_OBJ_DIR)/%.o: %.c $(UNITY_SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(UNITY_INC) -MMD -MP -c $< -o $@

$(TARGET_DYNAMIC): $(OBJECTS) $(COMMON_LIB) $(LIST_DYNAMIC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) \
		-L../common/build/lib -L../list/build/lib \
		$(LDLIBS) -llist \
		-Wl,-rpath,'$$ORIGIN/../../../list/build/lib'

$(TARGET_STATIC): $(OBJECTS) $(COMMON_LIB) $(LIST_STATIC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) \
		-L../common/build/lib $(LDLIBS) $(LIST_STATIC)

$(COMMON_LIB):
	$(MAKE) -C ../common

$(LIST_STATIC) $(LIST_DYNAMIC):
	$(MAKE) -C ../list

$(TEST_TARGET_DYNAMIC): $(TEST_OBJECTS) $(UNITY_OBJ) $(COMMON_LIB) $(LIST_DYNAMIC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJECTS) $(UNITY_OBJ) \
		-L../common/build/lib -L../list/build/lib \
		$(LDLIBS) -llist \
		-Wl,-rpath,'$$ORIGIN/../../../../list/build/lib'

$(TEST_TARGET_STATIC): $(TEST_OBJECTS) $(UNITY_OBJ) $(COMMON_LIB) $(LIST_STATIC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJECTS) $(UNITY_OBJ) \
		-L../common/build/lib $(LDLIBS) $(LIST_STATIC)

test: test-static test-dynamic
test-static: $(TEST_TARGET_STATIC)
	./$(TEST_TARGET_STATIC)
test-dynamic: $(TEST_TARGET_DYNAMIC)
	./$(TEST_TARGET_DYNAMIC)

run: $(TARGET_DYNAMIC)
	./$(TARGET_DYNAMIC) $(ARGS)

run-static: $(TARGET_STATIC)
	./$(TARGET_STATIC) $(ARGS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean test test-static test-dynamic run run-static
-include $(DEPS)
-include $(TEST_DEPS)