# Lightmap Update Test Makefile
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
TEST_UNOPT = test_lightmap_unopt
TEST_OPT = test_lightmap_opt

.PHONY: all test test_opt compare clean

all: $(TEST_UNOPT) $(TEST_OPT)

# Build unoptimized version test
$(TEST_UNOPT): test_lightmap.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Build optimized version test
$(TEST_OPT): test_lightmap.c
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
	@echo "PERFORMANCE COMPARISON: Full Recalc vs Dirty Rect"
	@echo "=============================================="
	@echo ""
	@echo ">>> UNOPTIMIZED VERSION (Full Recalculation) <<<"
	@./$(TEST_UNOPT) 2>&1 | grep -E "(Performance|Scaling|Testing:|ms avg|ms for|ratios)"
	@echo ""
	@echo ">>> OPTIMIZED VERSION (Dirty Rectangle Tracking) <<<"
	@./$(TEST_OPT) 2>&1 | grep -E "(Performance|Scaling|Testing:|ms avg|ms for|ratios)"
	@echo ""
	@echo "=============================================="
	@echo "Note: Optimized version benefits most from small lights"
	@echo "      that only affect a portion of the surface"
	@echo "=============================================="

clean:
	rm -f $(TEST_UNOPT) $(TEST_OPT)
	rm -f *.o
