# Makefile for julius-039 waypoint deletion test
#
# Compiles test that verifies proper array compaction on delete.
#
# Test validation:
# - FIXED code: Tests pass (array properly compacted)
# - BUGGY code: Tests fail (stale data in array)

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files
TEST_SRC = test_waypoint_delete.c
STUB_SRC = waypoint_stubs.c

# Test targets
TEST_BINARY = test_waypoint_delete

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test with stubs
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	@echo "Compiling waypoint deletion test..."
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests
# - On FIXED code: Tests pass (array compacted correctly)
# - On BUGGY code: Tests fail (stale waypoint data)
test: $(TEST_BINARY)
	@echo "Running waypoint deletion tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Waypoint deletion does not properly compact array."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
