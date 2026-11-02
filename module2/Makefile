.PHONY: all clean common calculator phonebook permissions packet_router priority_queue test unity format format-check install-hooks run run-calculator run-phonebook run-permissions run-packet_router run-priority_queue

UNITY_DIR      = third_party/Unity
UNITY_BASE_URL = https://raw.githubusercontent.com/ThrowTheSwitch/Unity/master/src

all: calculator phonebook permissions packet_router priority_queue

common:
	$(MAKE) -C common

calculator: common
	$(MAKE) -C calculator

phonebook: common
	$(MAKE) -C phonebook

permissions:
	$(MAKE) -C permissions

packet_router:
	$(MAKE) -C packet_router

priority_queue: common
	$(MAKE) -C priority_queue

test: common
	$(MAKE) -C common test
	$(MAKE) -C calculator test
	$(MAKE) -C phonebook test
	$(MAKE) -C permissions test
	$(MAKE) -C packet_router test
	$(MAKE) -C priority_queue test

run:
	@echo "Укажите приложение: make run-calculator | run-phonebook | run-permissions | run-packet_router | run-priority_queue"
	@echo "Параметры передаются через ARGS=\"...\": make run-phonebook ARGS=\"--collection tree\""

run-calculator: calculator
	./calculator/build/bin/calculator $(ARGS)

run-phonebook: phonebook
	./phonebook/build/bin/phonebook $(ARGS)

run-permissions: permissions
	./permissions/build/bin/permissions $(ARGS)

run-packet_router: packet_router
	./packet_router/build/bin/packet_router $(ARGS)

run-priority_queue: priority_queue
	./priority_queue/build/bin/priority_queue $(ARGS)

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
	$(MAKE) -C packet_router clean
	$(MAKE) -C priority_queue clean

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