# Makefile for julius-045 animation offset calculation test
#
# This test validates that animation_get_screen_position() calculates
# sprite positions correctly by applying offsets in the right order.
#
# Test validation:
# - FIXED code: frame offset added before sprite offset subtracted -> correct positions
# - BUGGY code: sprite offset subtracted first -> incorrect positions

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Test targets
TEST_BINARY = test_animation_offset
STUB_SRC = animation_stubs.c

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test against stub implementation
$(TEST_BINARY): test_animation_offset.c $(STUB_SRC)
	@echo "Compiling animation offset test..."
	$(CC) $(CFLAGS) -o $@ test_animation_offset.c $(STUB_SRC) $(LDFLAGS)

# Run tests
test: $(TEST_BINARY)
	@echo "Running animation offset tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Animation offsets are calculated in the wrong order."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
