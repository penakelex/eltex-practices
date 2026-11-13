# Требует: BUILD_DIR, SOURCES, TEST_SOURCES, TEST_TARGET,
#           common.mk подключён ДО lib.mk.
.DEFAULT_GOAL := all

OBJ_DIR       ?= $(BUILD_DIR)/obj
LIB_DIR       ?= $(BUILD_DIR)/lib
TEST_OBJ_DIR  ?= $(BUILD_DIR)/test/obj
TEST_BIN_DIR  ?= $(BUILD_DIR)/test/bin

OBJECTS      = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS         = $(OBJECTS:.o=.d)
TEST_OBJECTS = $(patsubst %.c,$(TEST_OBJ_DIR)/%.o,$(TEST_SOURCES))
TEST_DEPS    = $(TEST_OBJECTS:.o=.d)

.PHONY: all test clean

$(OBJ_DIR)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

$(TEST_OBJ_DIR)/%.o: %.c $(UNITY_SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(UNITY_INC) -MMD -MP -c $< -o $@

$(TEST_TARGET): $(TEST_OBJECTS) $(UNITY_OBJ)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJECTS) $(UNITY_OBJ)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
-include $(TEST_DEPS)