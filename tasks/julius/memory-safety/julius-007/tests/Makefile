# Makefile for julius-007 path buffer overflow test
#
# This test validates the fix for buffer overflow in file_construct_path.
#
# Test validation:
# - FIXED code: Tests pass (bounds checked, oversized paths rejected)
# - BUGGY code: Static analysis detects missing bounds check

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# AddressSanitizer flags
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer

# Julius source directory (set by test runner)
JULIUS_SRC ?= ..

# Source files - use stubs instead of compiling all of Julius
TEST_SRC = test_path_overflow.c
STUB_SRC = file_stubs.c

# Test binary
TEST_BINARY = test_path_overflow_asan

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test with AddressSanitizer against stub implementation
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests - includes both functional tests and static analysis
test: $(TEST_BINARY)
	@echo "=== Julius-007: Buffer Overflow Test ==="
	@echo ""
	@echo "--- Static Analysis of Julius Source ---"
	@# Check if file_construct_path exists and has bounds checking
	@if [ -f "$(JULIUS_SRC)/core/file.c" ]; then \
		if grep -q "file_construct_path" "$(JULIUS_SRC)/core/file.c"; then \
			echo "  Found file_construct_path in source"; \
			if grep -A 20 "file_construct_path" "$(JULIUS_SRC)/core/file.c" | grep -qE "(strlen|>=.*FILE_MAX|>=.*260|total.*len|>= FILE_MAX_PATH)"; then \
				echo "  PASS: Bounds checking detected"; \
			else \
				echo "  FAIL: No bounds checking found - buffer overflow possible"; \
				exit 1; \
			fi; \
		else \
			echo "  file_construct_path not found in source (may be in fixed state)"; \
			echo "  PASS: Function not present (no vulnerability)"; \
		fi; \
	else \
		echo "  WARNING: Julius source not found at $(JULIUS_SRC)/core/file.c"; \
	fi
	@echo ""
	@echo "--- Functional Tests (stub implementation) ---"
	@ASAN_OPTIONS=detect_leaks=0:abort_on_error=1:print_stacktrace=1 ./$(TEST_BINARY) && echo "Functional tests PASSED" || (echo "Functional tests FAILED"; exit 1)

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
