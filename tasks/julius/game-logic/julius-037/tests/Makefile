# Makefile for julius-037 tower archer range elevation test
#
# Compiles test that verifies 3D distance calculation.
#
# Test validation:
# - FIXED code: Tests pass (elevation affects range)
# - BUGGY code: Tests fail (2D distance only)

CC ?= clang
CFLAGS = -Wall -Wextra -g -lm
LDFLAGS = -lm

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files
TEST_SRC = test_archer_range.c
STUB_SRC = combat_stubs.c

# Test targets
TEST_BINARY = test_archer_range

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test with stubs
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	@echo "Compiling tower archer range elevation test..."
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests
# - On FIXED code: Tests pass (3D distance calculation)
# - On BUGGY code: Tests fail (elevation ignored)
test: $(TEST_BINARY)
	@echo "Running tower archer range elevation tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Tower archer range calculation ignores elevation."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
