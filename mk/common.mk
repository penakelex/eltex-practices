# Требует: ROOT, BUILD_DIR.
CC      := clang
AR      := ar
ARFLAGS := rcs
CFLAGS := -std=c23 -Wall -Wextra -O2

UNITY_DIR      := $(ROOT)/third_party/Unity
UNITY_BASE_URL := https://raw.githubusercontent.com/ThrowTheSwitch/Unity/master/src
UNITY_INC      := -I$(UNITY_DIR)/src
UNITY_SRC      := $(UNITY_DIR)/src/unity.c
UNITY_OBJ      ?= $(BUILD_DIR)/test/obj/unity.o

.PHONY: unity
unity: $(UNITY_SRC)

$(UNITY_SRC):
	mkdir -p $(UNITY_DIR)/src
	curl -fsSL $(UNITY_BASE_URL)/unity.c -o $(UNITY_DIR)/src/unity.c
	curl -fsSL $(UNITY_BASE_URL)/unity.h -o $(UNITY_DIR)/src/unity.h
	curl -fsSL $(UNITY_BASE_URL)/unity_internals.h -o $(UNITY_DIR)/src/unity_internals.h

$(UNITY_OBJ): $(UNITY_SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(UNITY_INC) -c $< -o $@

include $(dir $(lastword $(MAKEFILE_LIST)))format.mk