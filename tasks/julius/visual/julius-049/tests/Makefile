# Makefile for julius-049 cursor hotspot offset test
#
# This test validates that cursor_get_click_position() subtracts
# the hotspot offset instead of adding it.
#
# Test validation:
# - FIXED code: click_pos = cursor_pos - hotspot -> correct position
# - BUGGY code: click_pos = cursor_pos + hotspot -> wrong position

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Test targets
TEST_BINARY = test_cursor_hotspot
STUB_SRC = cursor_stubs.c

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test against stub implementation
$(TEST_BINARY): test_cursor_hotspot.c $(STUB_SRC)
	@echo "Compiling cursor hotspot test..."
	$(CC) $(CFLAGS) -o $@ test_cursor_hotspot.c $(STUB_SRC) $(LDFLAGS)

# Run tests
test: $(TEST_BINARY)
	@echo "Running cursor hotspot tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Cursor hotspot offset uses wrong sign (+ instead of -)."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
