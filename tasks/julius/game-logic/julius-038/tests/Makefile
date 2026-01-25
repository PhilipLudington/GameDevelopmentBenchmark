# Makefile for julius-038 granary cart pathfinding loop test
#
# Compiles test that verifies arrival check uses destination.
#
# Test validation:
# - FIXED code: Tests pass (arrival at destination detected)
# - BUGGY code: Tests fail (arrival never detected, infinite loop)

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files
TEST_SRC = test_cart_pathfinding.c
STUB_SRC = route_stubs.c

# Test targets
TEST_BINARY = test_cart_pathfinding

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test with stubs
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	@echo "Compiling granary cart pathfinding test..."
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests
# - On FIXED code: Tests pass (carts arrive at destination)
# - On BUGGY code: Tests fail (carts stuck in loop)
test: $(TEST_BINARY)
	@echo "Running granary cart pathfinding tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Granary cart pathfinding loop detected."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
