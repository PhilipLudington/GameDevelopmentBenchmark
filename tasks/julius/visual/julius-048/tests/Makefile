# Makefile for julius-048 scrollbar position rounding test
#
# This test validates that scrollbar_get_thumb_position() uses proper
# rounding instead of truncation to avoid visual jitter.
#
# Test validation:
# - FIXED code: Uses (a + b/2) / b for rounding -> smooth positions
# - BUGGY code: Uses a / b truncation -> jittery positions

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Test targets
TEST_BINARY = test_scrollbar_position
STUB_SRC = scrollbar_stubs.c

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test against stub implementation
$(TEST_BINARY): test_scrollbar_position.c $(STUB_SRC)
	@echo "Compiling scrollbar position test..."
	$(CC) $(CFLAGS) -o $@ test_scrollbar_position.c $(STUB_SRC) $(LDFLAGS)

# Run tests
test: $(TEST_BINARY)
	@echo "Running scrollbar position tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Scrollbar position uses truncation instead of rounding."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
