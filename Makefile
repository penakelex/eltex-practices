ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
MODULES := module2 module3

.PHONY: all clean test format format-check install-hooks run $(MODULES)
.PHONY: run-calculator run-phonebook run-phonebook-static \
        run-permissions run-packet_router run-priority_queue
.PHONY: run-01 run-02 run-03 run-04 run-05 run-06 run-07 run-08 \
        run-cross-01 run-cross-02

all: $(MODULES)

$(MODULES):
	$(MAKE) -C $@

test:
	@for m in $(MODULES); do $(MAKE) -C $$m test || exit 1; done

clean:
	@for m in $(MODULES); do $(MAKE) -C $$m clean || exit 1; done

include $(dir $(lastword $(MAKEFILE_LIST)))mk/format.mk

install-hooks:
	chmod +x scripts/pre-commit scripts/pre-push
	git config core.hooksPath scripts

run:
	@echo "Укажите приложение или задание:"
	@echo "  module2: make run-calculator | run-phonebook | run-phonebook-static"
	@echo "           make run-permissions | run-packet_router | run-priority_queue"
	@echo "  module3: make run-01"
	@echo "Параметры: ARGS=\"...\""

run-calculator run-phonebook run-permissions run-packet_router \
run-priority_queue run-phonebook-static: module2
	$(MAKE) -C module2 $@ ARGS="$(ARGS)"

run-01 run-02 run-03 run-04 run-05 run-06 run-07 run-08 \
run-cross-01 run-cross-02: module3
	$(MAKE) -C module3 $@ ARGS="$(ARGS)"