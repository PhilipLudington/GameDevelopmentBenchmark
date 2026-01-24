# Makefile for julius-001 smacker double-free test
#
# Compiles test against actual Julius smacker.c with ASan to detect double-free.
#
# The test creates a truncated SMK file that triggers the error path in
# read_frame_info(). On buggy code, this causes a double-free that ASan detects.
#
# Test validation:
# - FIXED code: Tests pass (no memory errors)
# - BUGGY code: ASan detects double-free, tests fail

CC ?= clang
CFLAGS = -Wall -Wextra -g -I$(JULIUS_SRC) -I$(JULIUS_SRC)/..
LDFLAGS =

# AddressSanitizer flags
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files
TEST_SRC = test_smacker_double_free.c
SMACKER_SRC = $(JULIUS_SRC)/core/smacker.c
STUB_SRC = julius_stubs.c

# Test targets
TEST_BINARY = test_smacker_double_free
TEST_BINARY_ASAN = test_smacker_double_free_asan

.PHONY: all clean test

all: $(TEST_BINARY_ASAN)

# Build test with AddressSanitizer against real smacker.c
$(TEST_BINARY_ASAN): $(TEST_SRC) $(STUB_SRC)
	@echo "Compiling test against Julius smacker.c with ASan..."
	@if [ ! -f "$(SMACKER_SRC)" ]; then \
		echo "Error: Julius source not found at $(SMACKER_SRC)"; \
		echo "Set JULIUS_SRC to the Julius src directory"; \
		exit 1; \
	fi
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -o $@ $(TEST_SRC) $(SMACKER_SRC) $(STUB_SRC) $(LDFLAGS)

# Build test without AddressSanitizer (for debugging)
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	@if [ ! -f "$(SMACKER_SRC)" ]; then \
		echo "Error: Julius source not found at $(SMACKER_SRC)"; \
		exit 1; \
	fi
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(SMACKER_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests with ASan
# - On FIXED code: Tests pass (no double-free)
# - On BUGGY code: ASan detects double-free, test fails
test: $(TEST_BINARY_ASAN)
	@echo "Running smacker double-free tests with ASan..."
	@echo ""
	@ASAN_OPTIONS="detect_leaks=0:abort_on_error=1:print_stacktrace=1" ./$(TEST_BINARY_ASAN); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "ASan likely detected a memory error (double-free)."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY) $(TEST_BINARY_ASAN)
	rm -rf *.dSYM
