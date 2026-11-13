# Требует: BUILD_DIR, SOURCES, TEST_SOURCES, TARGET, TEST_TARGET,
#           common.mk подключён ДО app.mk.
# Опционально: EXTRA_DEPS, EXTRA_TEST_DEPS.
.DEFAULT_GOAL := all

OBJ_DIR       ?= $(BUILD_DIR)/obj
BIN_DIR       ?= $(BUILD_DIR)/bin
TEST_OBJ_DIR  ?= $(BUILD_DIR)/test/obj
TEST_BIN_DIR  ?= $(BUILD_DIR)/test/bin

OBJECTS      = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS         = $(OBJECTS:.o=.d)
TEST_OBJECTS = $(patsubst %.c,$(TEST_OBJ_DIR)/%.o,$(TEST_SOURCES))
TEST_DEPS    = $(TEST_OBJECTS:.o=.d)

.PHONY: all test run clean

all: $(TARGET)

$(OBJ_DIR)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

$(TEST_OBJ_DIR)/%.o: %.c $(UNITY_SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(UNITY_INC) -MMD -MP -c $< -o $@

$(TARGET): $(OBJECTS) $(EXTRA_DEPS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS) $(LDLIBS)

$(TEST_TARGET): $(TEST_OBJECTS) $(UNITY_OBJ) $(EXTRA_TEST_DEPS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJECTS) $(UNITY_OBJ) $(LDFLAGS) $(LDLIBS)

test: $(TEST_TARGET) $(TARGET)
	./$(TEST_TARGET)

run: $(TARGET)
	./$(TARGET) $(ARGS)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
-include $(TEST_DEPS)