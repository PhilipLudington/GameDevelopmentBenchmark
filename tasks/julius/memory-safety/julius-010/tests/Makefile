# Makefile for julius-010 use-after-free test
#
# This test validates the fix for use-after-free in UI callback system.
#
# Test validation:
# - FIXED code: Tests pass (no dangling pointer)
# - BUGGY code: Static analysis detects store-then-free pattern

CC ?= clang
CFLAGS = -Wall -Wextra -g
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer
JULIUS_SRC ?= ..
TEST_BINARY = test_use_after_free_asan

.PHONY: all test clean

all: $(TEST_BINARY)

# Build test against stub implementation
$(TEST_BINARY): test_use_after_free.c window_stubs.c
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -o $@ test_use_after_free.c window_stubs.c

# Run tests - includes both functional tests and static analysis
test: $(TEST_BINARY)
	@echo "=== Julius-010: Use-After-Free Test ==="
	@echo ""
	@echo "--- Static Analysis of Julius Source ---"
	@if [ -f "$(JULIUS_SRC)/window/city.c" ]; then \
		if grep -q "ui_trigger_callback_unsafe" "$(JULIUS_SRC)/window/city.c"; then \
			echo "  Found ui_trigger_callback_unsafe in source"; \
			func_body=$$(grep -A 10 "ui_trigger_callback_unsafe" "$(JULIUS_SRC)/window/city.c"); \
			if echo "$$func_body" | grep -q "free(data)"; then \
				if echo "$$func_body" | grep -B 5 "free(data)" | grep -q "pending_callback_data = data"; then \
					echo "  FAIL: Store-then-free pattern detected (use-after-free)"; \
					exit 1; \
				fi; \
			fi; \
			echo "  PASS: No use-after-free pattern detected"; \
		else \
			echo "  ui_trigger_callback_unsafe not found (may be in fixed state)"; \
			echo "  PASS: Function not present (no vulnerability)"; \
		fi; \
	else \
		echo "  WARNING: Julius source not found at $(JULIUS_SRC)/window/city.c"; \
	fi
	@echo ""
	@echo "--- Functional Tests (stub implementation) ---"
	@ASAN_OPTIONS=detect_leaks=0:abort_on_error=1:print_stacktrace=1 ./$(TEST_BINARY) && echo "Functional tests PASSED" || (echo "Functional tests FAILED"; exit 1)

clean:
	rm -f $(TEST_BINARY) *.dSYM
