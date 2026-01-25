# Makefile for julius-034 palace upgrade validation test
#
# Compiles test that verifies palace upgrade boolean logic.
#
# Test validation:
# - FIXED code: Tests pass (upgrade denied when villa or palace exists)
# - BUGGY code: Tests fail (upgrade incorrectly allowed)

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files
TEST_SRC = test_palace_upgrade.c
STUB_SRC = palace_stubs.c

# Test targets
TEST_BINARY = test_palace_upgrade

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test with stubs
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	@echo "Compiling palace upgrade validation test..."
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests
# - On FIXED code: Tests pass (boolean logic correct)
# - On BUGGY code: Tests fail (OR instead of AND)
test: $(TEST_BINARY)
	@echo "Running palace upgrade validation tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Palace upgrade validation logic is incorrect."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
