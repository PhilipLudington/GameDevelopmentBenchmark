# Makefile for julius-035 popularity decay rounding test
#
# Compiles test that verifies integer math precision.
#
# Test validation:
# - FIXED code: Tests pass (correct decay calculation)
# - BUGGY code: Tests fail (truncation to zero or wrong values)

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files
TEST_SRC = test_popularity_decay.c
STUB_SRC = sentiment_stubs.c

# Test targets
TEST_BINARY = test_popularity_decay

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test with stubs
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	@echo "Compiling popularity decay rounding test..."
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests
# - On FIXED code: Tests pass (precision preserved)
# - On BUGGY code: Tests fail (integer truncation errors)
test: $(TEST_BINARY)
	@echo "Running popularity decay rounding tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Popularity decay calculation has precision errors."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
