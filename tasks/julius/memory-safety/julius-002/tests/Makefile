# Makefile for julius-002 dangling pointer test
#
# This test uses static analysis to verify that the localized_filename
# variable is declared in the correct scope in game/file.c.
#
# The bug: localized_filename is declared INSIDE the if block but used
# after the block ends - a dangling pointer that causes undefined behavior.
#
# Test validation:
# - FIXED code: localized_filename declared at function scope → test passes
# - BUGGY code: localized_filename declared inside if block → test fails

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Test targets
TEST_BINARY = test_dangling_pointer

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test (standalone, does static analysis of file.c)
$(TEST_BINARY): test_dangling_pointer.c
	@echo "Compiling dangling pointer static analysis test..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Run tests
# The test reads game/file.c and checks variable scope
test: $(TEST_BINARY)
	@echo "Running dangling pointer tests..."
	@echo ""
	@if [ ! -f "$(JULIUS_SRC)/game/file.c" ]; then \
		echo "Error: Julius source not found at $(JULIUS_SRC)/game/file.c"; \
		echo "Set JULIUS_SRC to the Julius src directory"; \
		exit 1; \
	fi
	@JULIUS_SRC="$(JULIUS_SRC)" ./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "localized_filename is declared in wrong scope (dangling pointer bug)."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
