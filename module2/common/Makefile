CC       = clang-23
CFLAGS   = -std=c23 -Wall -Wextra -O2 -DUNITY_INCLUDE_DOUBLE
CPPFLAGS = -Iinclude
AR       = ar
ARFLAGS  = rcs

UNITY_DIR      = ../third_party/Unity
UNITY_BASE_URL = https://raw.githubusercontent.com/ThrowTheSwitch/Unity/master/src
UNITY_INC      = -I$(UNITY_DIR)/src
UNITY_SRC      = $(UNITY_DIR)/src/unity.c

BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj
LIB_DIR   = $(BUILD_DIR)/lib

SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS    = $(OBJECTS:.o=.d)
LIBRARY = $(LIB_DIR)/libcommon.a

TEST_BUILD_DIR = $(BUILD_DIR)/test
TEST_OBJ_DIR   = $(TEST_BUILD_DIR)/obj
TEST_BIN_DIR   = $(TEST_BUILD_DIR)/bin
TEST_TARGET    = $(TEST_BIN_DIR)/test_common

TEST_SOURCES = $(wildcard tests/*.c) $(wildcard src/*.c)
TEST_OBJECTS = $(patsubst %.c,$(TEST_OBJ_DIR)/%.o,$(TEST_SOURCES))
TEST_DEPS    = $(TEST_OBJECTS:.o=.d)

UNITY_OBJ = $(TEST_OBJ_DIR)/unity.o

$(LIBRARY): $(OBJECTS)
	mkdir -p $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $^

$(OBJ_DIR)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(UNITY_SRC):
	mkdir -p $(UNITY_DIR)/src
	curl -fsSL $(UNITY_BASE_URL)/unity.c -o $(UNITY_DIR)/src/unity.c
	curl -fsSL $(UNITY_BASE_URL)/unity.h -o $(UNITY_DIR)/src/unity.h
	curl -fsSL $(UNITY_BASE_URL)/unity_internals.h -o $(UNITY_DIR)/src/unity_internals.h

$(UNITY_OBJ): $(UNITY_SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(UNITY_INC) -c $< -o $@

$(TEST_TARGET): $(TEST_OBJECTS) $(UNITY_OBJ)
	mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJECTS) $(UNITY_OBJ)

$(TEST_OBJ_DIR)/%.o: %.c $(UNITY_SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(UNITY_INC) -MMD -MP -c $< -o $@

-include $(DEPS)
-include $(TEST_DEPS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean test