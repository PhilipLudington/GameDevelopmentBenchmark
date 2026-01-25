# Makefile for julius-050 panel text truncation test
#
# This test validates that panel_get_text_width() subtracts
# padding from both sides (2x), not just once.
#
# Test validation:
# - FIXED code: Returns panel_width - (padding * 2) -> correct text width
# - BUGGY code: Returns panel_width - padding -> too wide, causes truncation

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Test targets
TEST_BINARY = test_panel_text_width
STUB_SRC = panel_stubs.c

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test against stub implementation
$(TEST_BINARY): test_panel_text_width.c $(STUB_SRC)
	@echo "Compiling panel text width test..."
	$(CC) $(CFLAGS) -o $@ test_panel_text_width.c $(STUB_SRC) $(LDFLAGS)

# Run tests
test: $(TEST_BINARY)
	@echo "Running panel text width tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Panel text width only subtracts padding once instead of twice."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
