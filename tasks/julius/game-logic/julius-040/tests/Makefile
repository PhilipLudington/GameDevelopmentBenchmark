# Makefile for julius-040 fireworks timing offset test
#
# Compiles test that verifies fireworks schedule after festival start.
#
# Test validation:
# - FIXED code: Tests pass (fireworks after festival start)
# - BUGGY code: Tests fail (fireworks before festival start)

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files
TEST_SRC = test_fireworks_timing.c
STUB_SRC = festival_stubs.c

# Test targets
TEST_BINARY = test_fireworks_timing

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test with stubs
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	@echo "Compiling fireworks timing test..."
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests
# - On FIXED code: Tests pass (fireworks at correct time)
# - On BUGGY code: Tests fail (fireworks too early)
test: $(TEST_BINARY)
	@echo "Running fireworks timing tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Fireworks timing calculation is incorrect."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
