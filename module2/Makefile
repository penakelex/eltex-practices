.PHONY: all clean common calculator phonebook permissions test unity format format-check install-hooks

UNITY_DIR      = third_party/Unity
UNITY_BASE_URL = https://raw.githubusercontent.com/ThrowTheSwitch/Unity/master/src

all: calculator phonebook permissions

common:
	$(MAKE) -C common

calculator: common
	$(MAKE) -C calculator

phonebook: common
	$(MAKE) -C phonebook

permissions:
	$(MAKE) -C permissions

test: common
	$(MAKE) -C common test
	$(MAKE) -C calculator test
	$(MAKE) -C phonebook test
	$(MAKE) -C permissions test

$(UNITY_DIR)/src/unity.c:
	mkdir -p $(UNITY_DIR)/src
	curl -fsSL $(UNITY_BASE_URL)/unity.c -o $(UNITY_DIR)/src/unity.c
	curl -fsSL $(UNITY_BASE_URL)/unity.h -o $(UNITY_DIR)/src/unity.h
	curl -fsSL $(UNITY_BASE_URL)/unity_internals.h -o $(UNITY_DIR)/src/unity_internals.h

unity: $(UNITY_DIR)/src/unity.c

clean:
	$(MAKE) -C common clean
	$(MAKE) -C calculator clean
	$(MAKE) -C phonebook clean
	$(MAKE) -C permissions clean

format:
	find . -type f \( -name '*.c' -o -name '*.h' \) \
	  -not -path './.git/*' \
	  -not -path '*/build/*' \
	  -not -path './third_party/*' -print0 \
	  | xargs -0 clang-format -i

format-check:
	find . -type f \( -name '*.c' -o -name '*.h' \) \
	  -not -path './.git/*' \
	  -not -path '*/build/*' \
	  -not -path './third_party/*' -print0 \
	  | xargs -0 clang-format --dry-run --Werror

install-hooks:
	chmod +x ../scripts/pre-commit ../scripts/pre-push
	cd .. && git config core.hooksPath scripts