# BSP Traversal Test Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

# Test with the buggy game code
test_buggy: test_bsp_traversal.c
	$(CC) $(CFLAGS) -DUSE_BUGGY_CODE -o test_bsp_buggy test_bsp_traversal.c
	@echo "Running tests with BUGGY code (should fail some tests):"
	@./test_bsp_buggy || true

# Test with the solution code
test_solution: test_bsp_traversal.c
	$(CC) $(CFLAGS) -DUSE_SOLUTION_CODE -o test_bsp_solution test_bsp_traversal.c
	@echo "Running tests with SOLUTION code (should pass all tests):"
	@./test_bsp_solution

# Default: run both
test: test_buggy test_solution

clean:
	rm -f test_bsp_buggy test_bsp_solution

.PHONY: test test_buggy test_solution clean
