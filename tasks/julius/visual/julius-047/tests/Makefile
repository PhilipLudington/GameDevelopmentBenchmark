# Makefile for julius-047 button highlight color test
#
# This test validates that button_get_highlight_color() returns
# the correct theme color (COLOR_BUTTON_HIGHLIGHT, not COLOR_BUTTON_BORDER).
#
# Test validation:
# - FIXED code: Returns color at index 4 (highlight) -> correct color
# - BUGGY code: Returns color at index 3 (border) -> wrong color

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Test targets
TEST_BINARY = test_button_highlight
STUB_SRC = button_stubs.c

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test against stub implementation
$(TEST_BINARY): test_button_highlight.c $(STUB_SRC)
	@echo "Compiling button highlight color test..."
	$(CC) $(CFLAGS) -o $@ test_button_highlight.c $(STUB_SRC) $(LDFLAGS)

# Run tests
test: $(TEST_BINARY)
	@echo "Running button highlight color tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Button highlight uses COLOR_BUTTON_BORDER instead of COLOR_BUTTON_HIGHLIGHT."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
