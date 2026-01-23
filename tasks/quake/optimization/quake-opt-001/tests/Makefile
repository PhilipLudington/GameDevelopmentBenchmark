# PVS Decompression Test Makefile
#
# Build targets:
#   make test          - Run tests on unoptimized version
#   make test_opt      - Run tests on optimized version
#   make test_cache    - Run tests on optimized version with cache enabled
#   make compare       - Run both and compare performance
#   make clean         - Clean build artifacts

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS = -lm

# Test executables
TEST_UNOPT = test_pvs_unopt
TEST_OPT = test_pvs_opt
TEST_CACHE = test_pvs_cache

.PHONY: all test test_opt test_cache compare clean

all: $(TEST_UNOPT) $(TEST_OPT) $(TEST_CACHE)

# Build unoptimized version test
$(TEST_UNOPT): test_pvs.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Build optimized version test (cache disabled for fair comparison)
$(TEST_OPT): test_pvs.c
	$(CC) $(CFLAGS) -DTEST_OPTIMIZED -o $@ $< $(LDFLAGS)

# Build optimized version test with cache enabled
$(TEST_CACHE): test_pvs.c
	$(CC) $(CFLAGS) -DTEST_OPTIMIZED -DENABLE_CACHE -o $@ $< $(LDFLAGS)

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

# Run optimized tests with cache
test_cache: $(TEST_CACHE)
	@echo "Running tests on OPTIMIZED version with CACHE..."
	@echo ""
	@./$(TEST_CACHE)

# Compare both versions
compare: $(TEST_UNOPT) $(TEST_OPT) $(TEST_CACHE)
	@echo "=============================================="
	@echo "PERFORMANCE COMPARISON"
	@echo "=============================================="
	@echo ""
	@echo ">>> UNOPTIMIZED VERSION <<<"
	@./$(TEST_UNOPT) 2>&1 | grep -E "(Performance|Testing:|us avg)"
	@echo ""
	@echo ">>> OPTIMIZED VERSION (no cache) <<<"
	@./$(TEST_OPT) 2>&1 | grep -E "(Performance|Testing:|us avg)"
	@echo ""
	@echo ">>> OPTIMIZED VERSION (with cache) <<<"
	@./$(TEST_CACHE) 2>&1 | grep -E "(Performance|Testing:|Cache:|us avg)"
	@echo ""
	@echo "=============================================="

clean:
	rm -f $(TEST_UNOPT) $(TEST_OPT) $(TEST_CACHE)
	rm -f *.o
