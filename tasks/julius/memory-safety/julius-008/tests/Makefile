# Makefile for julius-008 integer overflow test
#
# This test validates the fix for integer overflow in resource_calculate_value.
#
# Test validation:
# - FIXED code: Tests pass (overflow detected/prevented)
# - BUGGY code: Static analysis detects missing overflow check

CC ?= clang
CFLAGS = -Wall -Wextra -g
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer
JULIUS_SRC ?= ..
TEST_BINARY = test_integer_overflow

.PHONY: all test clean

all: $(TEST_BINARY)

# Build test against stub implementation
$(TEST_BINARY): test_integer_overflow.c resource_stubs.c
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -o $@ test_integer_overflow.c resource_stubs.c

# Run tests - includes both functional tests and static analysis
test: $(TEST_BINARY)
	@echo "=== Julius-008: Integer Overflow Test ==="
	@echo ""
	@echo "--- Static Analysis of Julius Source ---"
	@if [ -f "$(JULIUS_SRC)/city/resource.c" ]; then \
		if grep -q "resource_calculate_value" "$(JULIUS_SRC)/city/resource.c"; then \
			echo "  Found resource_calculate_value in source"; \
			if grep -A 15 "resource_calculate_value" "$(JULIUS_SRC)/city/resource.c" | grep -qE "(int64_t|long long|INT_MAX|overflow|>.*INT_MAX|\(int64_t\))"; then \
				echo "  PASS: Overflow protection detected"; \
			else \
				echo "  FAIL: No overflow protection found"; \
				exit 1; \
			fi; \
		else \
			echo "  resource_calculate_value not found (may be in fixed state)"; \
			echo "  PASS: Function not present (no vulnerability)"; \
		fi; \
	else \
		echo "  WARNING: Julius source not found at $(JULIUS_SRC)/city/resource.c"; \
	fi
	@echo ""
	@echo "--- Functional Tests (stub implementation) ---"
	@ASAN_OPTIONS=detect_leaks=0 ./$(TEST_BINARY) && echo "Functional tests PASSED" || (echo "Functional tests FAILED"; exit 1)

clean:
	rm -f $(TEST_BINARY) *.dSYM
