# Zone Allocator Test Makefile
#
# Build targets:
#   make test          - Run tests on buggy (game) version
#   make test_solution - Run tests on fixed (solution) version
#   make compare       - Run both and compare results
#   make clean         - Clean build artifacts

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS =

# Test executables
TEST_GAME = test_zone_game
TEST_SOLUTION = test_zone_solution

.PHONY: all test test_solution compare clean

all: $(TEST_GAME) $(TEST_SOLUTION)

# Build game (buggy) version test
$(TEST_GAME): test_zone.c ../game/zone.c ../game/quakedef.h
	$(CC) $(CFLAGS) -o $@ test_zone.c $(LDFLAGS)

# Build solution (fixed) version test
$(TEST_SOLUTION): test_zone.c ../solution/zone.c ../solution/quakedef.h
	$(CC) $(CFLAGS) -DTEST_SOLUTION -o $@ test_zone.c $(LDFLAGS)

# Run tests on buggy version (expected to fail some tests)
test: $(TEST_GAME)
	@echo "Running tests on BUGGY (game) version..."
	@echo "Note: Some tests are expected to fail."
	@echo ""
	@./$(TEST_GAME) || true

# Run tests on fixed version (all should pass)
test_solution: $(TEST_SOLUTION)
	@echo "Running tests on FIXED (solution) version..."
	@echo ""
	@./$(TEST_SOLUTION)

# Compare both versions
compare: $(TEST_GAME) $(TEST_SOLUTION)
	@echo "=============================================="
	@echo "COMPARISON: GAME vs SOLUTION"
	@echo "=============================================="
	@echo ""
	@echo ">>> BUGGY (game) VERSION <<<"
	@./$(TEST_GAME) 2>&1 | tail -5
	@echo ""
	@echo ">>> FIXED (solution) VERSION <<<"
	@./$(TEST_SOLUTION) 2>&1 | tail -5
	@echo ""
	@echo "=============================================="
	@echo "The game version should FAIL several tests."
	@echo "The solution version should PASS all tests."
	@echo "=============================================="

clean:
	rm -f $(TEST_GAME) $(TEST_SOLUTION)
	rm -f *.o
