# Makefile for julius-043 prefect message queue test
#
# Compiles test that verifies circular queue wraparound.
#
# Test validation:
# - FIXED code: Tests pass (queue wraps correctly)
# - BUGGY code: Tests fail (tail grows unbounded or wraps incorrectly)

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files
TEST_SRC = test_prefect_queue.c
STUB_SRC = prefect_stubs.c

# Test targets
TEST_BINARY = test_prefect_queue

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test with stubs
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	@echo "Compiling prefect message queue test..."
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests
# - On FIXED code: Tests pass (circular buffer works correctly)
# - On BUGGY code: Tests fail (incorrect wraparound)
test: $(TEST_BINARY)
	@echo "Running prefect message queue tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Prefect message queue wraparound is incorrect."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
