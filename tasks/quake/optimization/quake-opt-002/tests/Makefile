# Spatial Hash / Entity Collision Test Makefile
#
# Build targets:
#   make test          - Run tests on unoptimized version
#   make test_opt      - Run tests on optimized version
#   make compare       - Run both and compare performance
#   make clean         - Clean build artifacts

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS = -lm

# Test executables
TEST_UNOPT = test_world_unopt
TEST_OPT = test_world_opt

.PHONY: all test test_opt compare clean

all: $(TEST_UNOPT) $(TEST_OPT)

# Build unoptimized version test
$(TEST_UNOPT): test_spatial_hash.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Build optimized version test
$(TEST_OPT): test_spatial_hash.c
	$(CC) $(CFLAGS) -DTEST_OPTIMIZED -o $@ $< $(LDFLAGS)

# Run unoptimized tests
test: $(TEST_UNOPT)
	@echo "Running tests on UNOPTIMIZED version..."
	@echo ""
	@./$(TEST_UNOPT)

# Run optimized tests
test_opt: $(TEST_OPT)
	@echo "Running tests on OPTIMIZED version..."
	@echo ""
	@./$(TEST_OPT)

# Compare both versions
compare: $(TEST_UNOPT) $(TEST_OPT)
	@echo "=============================================="
	@echo "PERFORMANCE COMPARISON: Linked List vs Spatial Hash"
	@echo "=============================================="
	@echo ""
	@echo ">>> UNOPTIMIZED VERSION (Linked List - O(n²)) <<<"
	@./$(TEST_UNOPT) 2>&1 | grep -E "(Performance|Scaling|Testing:|ms avg|ratios)"
	@echo ""
	@echo ">>> OPTIMIZED VERSION (Spatial Hash - O(n)) <<<"
	@./$(TEST_OPT) 2>&1 | grep -E "(Performance|Scaling|Testing:|ms avg|ratios)"
	@echo ""
	@echo "=============================================="
	@echo "Note: O(n²) shows ~4x slowdown for 2x entities"
	@echo "      O(n) shows ~2x slowdown for 2x entities"
	@echo "=============================================="

clean:
	rm -f $(TEST_UNOPT) $(TEST_OPT)
	rm -f *.o
