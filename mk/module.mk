# Требует: LIBS, APPS (могут быть пустыми), BUILT = LIBS + APPS.
BUILT ?= $(LIBS) $(APPS)

.PHONY: all test clean run
.PHONY: $(BUILT)
.PHONY: $(addprefix test-,$(BUILT))

all: $(APPS)

$(BUILT):
	$(MAKE) -C $@

test: $(addprefix test-,$(BUILT))

$(addprefix test-,$(BUILT)):
	$(MAKE) -C $(patsubst test-%,%,$@) test

clean:
	@for d in $(BUILT); do \
	  [ -f $$d/Makefile ] && $(MAKE) -C $$d clean || true; \
	done

run-%:
	$(MAKE) -C $* run ARGS="$(ARGS)"

include $(dir $(lastword $(MAKEFILE_LIST)))format.mk