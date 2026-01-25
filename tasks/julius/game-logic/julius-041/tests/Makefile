# Makefile for julius-041 formation rank overflow test
#
# Compiles test that verifies array bounds checking.
#
# Test validation:
# - FIXED code: Tests pass (bounds respected, no overflow)
# - BUGGY code: Tests fail or crash (array overflow)

CC ?= clang
CFLAGS = -Wall -Wextra -g -fsanitize=address -fno-omit-frame-pointer
LDFLAGS = -fsanitize=address

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files
TEST_SRC = test_formation_rank.c
STUB_SRC = formation_stubs.c

# Test targets
TEST_BINARY = test_formation_rank

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test with stubs and ASAN
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	@echo "Compiling formation rank overflow test (with ASAN)..."
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests
# - On FIXED code: Tests pass (bounds checking prevents overflow)
# - On BUGGY code: ASAN detects buffer overflow
test: $(TEST_BINARY)
	@echo "Running formation rank overflow tests..."
	@echo ""
	@ASAN_OPTIONS=detect_leaks=0:abort_on_error=1 ./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Formation rank array overflow detected."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
