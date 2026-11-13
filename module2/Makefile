ROOT := $(abspath ..)

LIBS := common list
APPS := calculator phonebook permissions packet_router priority_queue
BUILT := $(LIBS) $(APPS)

include $(ROOT)/mk/module.mk

run-phonebook-static:
	$(MAKE) -C phonebook run-static ARGS="$(ARGS)"

run:
	@echo "Укажите приложение: make run-<app> ARGS=\"...\""
	@echo "Доступные: $(addprefix run-,$(APPS)) run-phonebook-static"

.PHONY: run run-phonebook-static