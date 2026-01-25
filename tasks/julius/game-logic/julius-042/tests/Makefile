# Makefile for julius-042 caravan follower index test
#
# Compiles test that verifies all followers are updated.
#
# Test validation:
# - FIXED code: Tests pass (all followers updated, including index 0)
# - BUGGY code: Tests fail (first follower skipped)

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files
TEST_SRC = test_caravan_follower.c
STUB_SRC = trader_stubs.c

# Test targets
TEST_BINARY = test_caravan_follower

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test with stubs
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	@echo "Compiling caravan follower index test..."
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests
# - On FIXED code: Tests pass (all followers processed)
# - On BUGGY code: Tests fail (first follower not updated)
test: $(TEST_BINARY)
	@echo "Running caravan follower index tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Caravan follower update loop has off-by-one error."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
