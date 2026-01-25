# Makefile for julius-033 storage building undo linking test
#
# Compiles test that verifies doubly-linked list integrity after removal.
#
# Test validation:
# - FIXED code: Tests pass (list integrity maintained after removal)
# - BUGGY code: Tests fail (prev pointer corruption detected)

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files
TEST_SRC = test_storage_undo.c
STUB_SRC = storage_stubs.c

# Test targets
TEST_BINARY = test_storage_undo

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test with stubs
$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	@echo "Compiling storage building undo test..."
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

# Run tests
# - On FIXED code: Tests pass (linked list properly maintained)
# - On BUGGY code: Tests fail (prev pointer corruption)
test: $(TEST_BINARY)
	@echo "Running storage building undo tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Storage building linked list corruption detected."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
