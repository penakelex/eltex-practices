ROOT := $(abspath ../..)
BUILD_DIR := build
CPPFLAGS := -Iinclude

LIBRARY_STATIC  := $(BUILD_DIR)/lib/liblist.a
LIBRARY_DYNAMIC := $(BUILD_DIR)/lib/liblist.so
SOURCES := $(wildcard src/*.c)
TEST_SOURCES := $(wildcard tests/*.c) $(wildcard src/*.c)
TEST_TARGET := $(BUILD_DIR)/test/bin/test_list

all: $(LIBRARY_STATIC) $(LIBRARY_DYNAMIC)

include $(ROOT)/mk/common.mk
CFLAGS += -fPIC
include $(ROOT)/mk/lib.mk

static: $(LIBRARY_STATIC)
dynamic: $(LIBRARY_DYNAMIC)

$(LIBRARY_STATIC): $(OBJECTS)
	mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $^

$(LIBRARY_DYNAMIC): $(OBJECTS)
	mkdir -p $(dir $@)
	$(CC) -shared -Wl,-soname,liblist.so -o $@ $^

.PHONY: static dynamic