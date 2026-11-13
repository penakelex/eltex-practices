ifeq ($(FORMAT_MK_INCLUDED),)
FORMAT_MK_INCLUDED := 1

.PHONY: format format-check

FORMAT_FIND = find . -type f \( -name '*.c' -o -name '*.h' \) \
  -not -path './.git/*' \
  -not -path '*/build/*' \
  -not -path './third_party/*'

format:
	$(FORMAT_FIND) -print0 | xargs -0 -r clang-format -i

format-check:
	$(FORMAT_FIND) -print0 | xargs -0 -r clang-format --dry-run --Werror

endif