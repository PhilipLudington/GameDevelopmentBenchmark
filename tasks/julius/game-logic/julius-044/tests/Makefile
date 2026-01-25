# Makefile for julius-044 building cost refund test
#
# Compiles test that verifies refund uses actual construction cost.
#
# Test validation:
# - FIXED code: Tests pass (refund based on actual cost paid)
# - BUGGY code: Tests fail (refund based on base cost only)

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files
TEST_SRC = test_refund_calculation.c
STUB_SRC = construction_stubs.c

# Test targets
TEST_BINARY = test_refund_calculation

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test with stubs
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	@echo "Compiling building cost refund test..."
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests
# - On FIXED code: Tests pass (refund matches construction cost)
# - On BUGGY code: Tests fail (refund ignores terrain modifiers)
test: $(TEST_BINARY)
	@echo "Running building cost refund tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Building cost refund calculation is incorrect."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
