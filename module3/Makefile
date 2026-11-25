ROOT := $(abspath ..)

LIBS := 04-05 06-07
APPS := 01 02 03 04 05 06 07
BUILT := $(LIBS) $(APPS)

include $(ROOT)/mk/module.mk

run-07-server:
	$(MAKE) -C 07 run-server ARGS="$(ARGS)"

run:
	@echo "Укажите задание: make run-01 ARGS=\"...\""