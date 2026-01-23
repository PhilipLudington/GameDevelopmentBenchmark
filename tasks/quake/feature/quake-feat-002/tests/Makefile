# Portal Culling Test Makefile
#
# Build targets:
#   make test          - Run tests on incomplete (game) version
#   make test_solution - Run tests on complete (solution) version
#   make compare       - Run both and compare results
#   make clean         - Clean build artifacts

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS = -lm

# Test executables
TEST_GAME = test_portals_game
TEST_SOLUTION = test_portals_solution

.PHONY: all test test_solution compare clean

all: $(TEST_GAME) $(TEST_SOLUTION)

# Build game (incomplete) version test
$(TEST_GAME): test_portals.c ../game/portal.c ../game/quakedef.h
	$(CC) $(CFLAGS) -o $@ test_portals.c $(LDFLAGS)

# Build solution (complete) version test
$(TEST_SOLUTION): test_portals.c ../solution/portal.c ../solution/quakedef.h
	$(CC) $(CFLAGS) -DTEST_SOLUTION -o $@ test_portals.c $(LDFLAGS)

# Run tests on incomplete version (expected to fail some tests)
test: $(TEST_GAME)
	@echo "Running tests on INCOMPLETE (game) version..."
	@echo "Note: Some tests are expected to fail."
	@echo ""
	@./$(TEST_GAME) || true

# Run tests on complete version (all should pass)
test_solution: $(TEST_SOLUTION)
	@echo "Running tests on COMPLETE (solution) version..."
	@echo ""
	@./$(TEST_SOLUTION)

# Compare both versions
compare: $(TEST_GAME) $(TEST_SOLUTION)
	@echo "=============================================="
	@echo "COMPARISON: GAME vs SOLUTION"
	@echo "=============================================="
	@echo ""
	@echo ">>> INCOMPLETE (game) VERSION <<<"
	@./$(TEST_GAME) 2>&1 | tail -5
	@echo ""
	@echo ">>> COMPLETE (solution) VERSION <<<"
	@./$(TEST_SOLUTION) 2>&1 | tail -5
	@echo ""
	@echo "=============================================="
	@echo "The game version should FAIL several tests."
	@echo "The solution version should PASS all tests."
	@echo "=============================================="

clean:
	rm -f $(TEST_GAME) $(TEST_SOLUTION)
	rm -f *.o
