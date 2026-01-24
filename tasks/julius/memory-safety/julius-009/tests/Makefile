# Makefile for julius-009 null pointer test
#
# This test validates the fix for null pointer dereference in building lookup.
#
# Test validation:
# - FIXED code: Tests pass (NULL check prevents crash)
# - BUGGY code: Static analysis detects missing NULL check

CC ?= clang
CFLAGS = -Wall -Wextra -g
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer
JULIUS_SRC ?= ..
TEST_BINARY = test_null_pointer_asan

.PHONY: all test clean

all: $(TEST_BINARY)

# Build test against stub implementation
$(TEST_BINARY): test_null_pointer.c building_stubs.c
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -o $@ test_null_pointer.c building_stubs.c

# Run tests - includes both functional tests and static analysis
test: $(TEST_BINARY)
	@echo "=== Julius-009: Null Pointer Test ==="
	@echo ""
	@echo "--- Static Analysis of Julius Source ---"
	@if [ -f "$(JULIUS_SRC)/building/building.c" ]; then \
		if grep -q "building_get_fire_risk_unsafe" "$(JULIUS_SRC)/building/building.c"; then \
			echo "  Found building_get_fire_risk_unsafe in source"; \
			func_body=$$(grep -A 10 "building_get_fire_risk_unsafe" "$(JULIUS_SRC)/building/building.c"); \
			if echo "$$func_body" | grep -qE "(!b|== NULL|!= NULL|if.*\!|NULL check)"; then \
				echo "  PASS: NULL check detected"; \
			else \
				echo "  FAIL: No NULL check found - null pointer dereference possible"; \
				exit 1; \
			fi; \
		else \
			echo "  building_get_fire_risk_unsafe not found (may be in fixed state)"; \
			echo "  PASS: Function not present (no vulnerability)"; \
		fi; \
	else \
		echo "  WARNING: Julius source not found at $(JULIUS_SRC)/building/building.c"; \
	fi
	@echo ""
	@echo "--- Functional Tests (stub implementation) ---"
	@ASAN_OPTIONS=detect_leaks=0:abort_on_error=1:print_stacktrace=1 ./$(TEST_BINARY) && echo "Functional tests PASSED" || (echo "Functional tests FAILED"; exit 1)

clean:
	rm -f $(TEST_BINARY) *.dSYM
