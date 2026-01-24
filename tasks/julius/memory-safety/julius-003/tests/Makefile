# Makefile for julius-003 sheep out-of-bounds test
#
# Compiles test against actual Julius animal.c with ASan to detect OOB access.
#
# The test places a herd formation at the map corner and verifies that animal
# destinations are properly bounded. On buggy code, coordinates exceed map
# bounds and ASan detects the out-of-bounds memory access.
#
# Test validation:
# - FIXED code: Tests pass (coordinates bounded by map_grid_bound())
# - BUGGY code: ASan detects heap-buffer-overflow, tests fail

CC ?= clang
CFLAGS = -Wall -Wextra -g -I$(JULIUS_SRC) -I$(JULIUS_SRC)/..
LDFLAGS =

# AddressSanitizer flags
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files
TEST_SRC = test_sheep_oob.c
STUB_SRC = julius_stubs.c
ANIMAL_SRC = $(JULIUS_SRC)/figuretype/animal.c

# Test targets
TEST_BINARY = test_sheep_oob
TEST_BINARY_ASAN = test_sheep_oob_asan

.PHONY: all clean test

all: $(TEST_BINARY_ASAN)

# Build test with AddressSanitizer against real animal.c
$(TEST_BINARY_ASAN): $(TEST_SRC)
	@echo "Compiling test against Julius animal.c with ASan..."
	@if [ ! -f "$(ANIMAL_SRC)" ]; then \
		echo "Error: Julius source not found at $(ANIMAL_SRC)"; \
		echo "Set JULIUS_SRC to the Julius src directory"; \
		exit 1; \
	fi
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -o $@ $(TEST_SRC) $(ANIMAL_SRC) $(STUB_SRC) $(LDFLAGS)

# Build test without AddressSanitizer (for debugging)
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	@if [ ! -f "$(ANIMAL_SRC)" ]; then \
		echo "Error: Julius source not found at $(ANIMAL_SRC)"; \
		exit 1; \
	fi
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(ANIMAL_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests with ASan
# - On FIXED code: Tests pass (coordinates bounded)
# - On BUGGY code: ASan detects OOB access, test fails
test: $(TEST_BINARY_ASAN)
	@echo "Running sheep OOB tests with ASan..."
	@echo ""
	@ASAN_OPTIONS="detect_leaks=0:abort_on_error=1:print_stacktrace=1" ./$(TEST_BINARY_ASAN); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "ASan likely detected a memory error (heap-buffer-overflow)."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY) $(TEST_BINARY_ASAN)
	rm -rf *.dSYM
