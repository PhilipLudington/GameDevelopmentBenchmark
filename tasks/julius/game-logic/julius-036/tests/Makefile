# Makefile for julius-036 plague spread rate integer overflow test
#
# Compiles test that verifies overflow-safe multiplication.
#
# Test validation:
# - FIXED code: Tests pass (overflow prevented, correct capping)
# - BUGGY code: Tests fail (overflow causes wrong values)

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files
TEST_SRC = test_plague_spread.c
STUB_SRC = health_stubs.c

# Test targets
TEST_BINARY = test_plague_spread

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test with stubs
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	@echo "Compiling plague spread rate overflow test..."
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests
# - On FIXED code: Tests pass (overflow-safe calculation)
# - On BUGGY code: Tests fail (integer overflow)
test: $(TEST_BINARY)
	@echo "Running plague spread rate overflow tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Plague spread calculation has integer overflow issues."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
