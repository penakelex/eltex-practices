CC       = clang
CFLAGS   = -std=c23 -Wall -Wextra -O2 -DUNITY_INCLUDE_DOUBLE
CPPFLAGS = -Iinclude -I../common/include -I../list/include
LDLIBS   = -lcommon

UNITY_DIR      = ../third_party/Unity
UNITY_BASE_URL = https://raw.githubusercontent.com/ThrowTheSwitch/Unity/master/src
UNITY_INC      = -I$(UNITY_DIR)/src
UNITY_SRC      = $(UNITY_DIR)/src/unity.c

BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj
BIN_DIR   = $(BUILD_DIR)/bin

SOURCES = $(wildcard src/*.c) $(wildcard src/collections/*.c)
OBJECTS = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS    = $(OBJECTS:.o=.d)

COMMON_LIB   = ../common/build/lib/libcommon.a
LIST_STATIC  = ../list/build/lib/liblist.a
LIST_DYNAMIC = ../list/build/lib/liblist.so

TARGET_DYNAMIC = $(BIN_DIR)/phonebook
TARGET_STATIC  = $(BIN_DIR)/phonebook-static

TEST_BUILD_DIR = $(BUILD_DIR)/test
TEST_OBJ_DIR   = $(TEST_BUILD_DIR)/obj
TEST_BIN_DIR   = $(TEST_BUILD_DIR)/bin
TEST_TARGET_DYNAMIC = $(TEST_BIN_DIR)/test_phonebook
TEST_TARGET_STATIC  = $(TEST_BIN_DIR)/test_phonebook-static

TEST_SOURCES = $(wildcard tests/*.c) \
			   $(filter-out src/main.c,$(wildcard src/*.c)) \
			   $(wildcard src/collections/*.c)
TEST_OBJECTS = $(patsubst %.c,$(TEST_OBJ_DIR)/%.o,$(TEST_SOURCES))
TEST_DEPS    = $(TEST_OBJECTS:.o=.d)

UNITY_OBJ = $(TEST_OBJ_DIR)/unity.o

all: $(TARGET_DYNAMIC) $(TARGET_STATIC)

$(TARGET_DYNAMIC): $(OBJECTS) $(COMMON_LIB) $(LIST_DYNAMIC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) \
		-L../common/build/lib -L../list/build/lib \
		$(LDLIBS) -llist \
		-Wl,-rpath,'$$ORIGIN/../../../list/build/lib'

$(TARGET_STATIC): $(OBJECTS) $(COMMON_LIB) $(LIST_STATIC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) \
		-L../common/build/lib \
		$(LDLIBS) $(LIST_STATIC)

$(COMMON_LIB):
	$(MAKE) -C ../common

$(LIST_STATIC) $(LIST_DYNAMIC):
	$(MAKE) -C ../list

$(OBJ_DIR)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

test: test-static test-dynamic

test-static: $(TEST_TARGET_STATIC)
	./$(TEST_TARGET_STATIC)

test-dynamic: $(TEST_TARGET_DYNAMIC)
	./$(TEST_TARGET_DYNAMIC)

run: $(TARGET_DYNAMIC)
	./$(TARGET_DYNAMIC) $(ARGS)

run-static: $(TARGET_STATIC)
	./$(TARGET_STATIC) $(ARGS)

$(UNITY_SRC):
	mkdir -p $(UNITY_DIR)/src
	curl -fsSL $(UNITY_BASE_URL)/unity.c -o $(UNITY_DIR)/src/unity.c
	curl -fsSL $(UNITY_BASE_URL)/unity.h -o $(UNITY_DIR)/src/unity.h
	curl -fsSL $(UNITY_BASE_URL)/unity_internals.h -o $(UNITY_DIR)/src/unity_internals.h

$(UNITY_OBJ): $(UNITY_SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(UNITY_INC) -c $< -o $@

$(TEST_TARGET_DYNAMIC): $(TEST_OBJECTS) $(UNITY_OBJ) $(COMMON_LIB) $(LIST_DYNAMIC)
	mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJECTS) $(UNITY_OBJ) \
		-L../common/build/lib -L../list/build/lib \
		$(LDLIBS) -llist \
		-Wl,-rpath,'$$ORIGIN/../../../../list/build/lib'

$(TEST_TARGET_STATIC): $(TEST_OBJECTS) $(UNITY_OBJ) $(COMMON_LIB) $(LIST_STATIC)
	mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJECTS) $(UNITY_OBJ) \
		-L../common/build/lib \
		$(LDLIBS) $(LIST_STATIC)

$(TEST_OBJ_DIR)/%.o: %.c $(UNITY_SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(UNITY_INC) -MMD -MP -c $< -o $@

-include $(DEPS)
-include $(TEST_DEPS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean test test-static test-dynamic run run-static