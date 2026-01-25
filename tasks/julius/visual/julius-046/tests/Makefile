# Makefile for julius-046 amphitheater spectator position test
#
# This test validates that figure_draw_spectator() uses the correct
# tile-to-pixel multiplier for both X and Y coordinates.
#
# Test validation:
# - FIXED code: Uses 15 for both X and Y -> spectators positioned correctly
# - BUGGY code: Uses 30 for X -> spectators shifted horizontally

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Test targets
TEST_BINARY = test_spectator_position
STUB_SRC = spectator_stubs.c

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test against stub implementation
$(TEST_BINARY): test_spectator_position.c $(STUB_SRC)
	@echo "Compiling spectator position test..."
	$(CC) $(CFLAGS) -o $@ test_spectator_position.c $(STUB_SRC) $(LDFLAGS)

# Run tests
test: $(TEST_BINARY)
	@echo "Running spectator position tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Spectator X coordinate uses wrong multiplier (30 instead of 15)."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
