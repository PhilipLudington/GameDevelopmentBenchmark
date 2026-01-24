# Makefile for julius-005 hotkey ordering test
#
# Compiles test that verifies enum-to-string mapping in hotkey_config.c.
# The test #includes the actual hotkey_config.c to access the static ini_keys[].
#
# Test validation:
# - FIXED code: Tests pass (ini_keys[11]="build_clear_land")
# - BUGGY code: Tests fail (ini_keys[11]="build_vacant_house" - WRONG!)

CC ?= clang
CFLAGS = -Wall -Wextra -g -I$(JULIUS_SRC) -I$(JULIUS_SRC)/..
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files - test #includes hotkey_config.c directly for static access
TEST_SRC = test_hotkey_ordering.c

# Test targets
TEST_BINARY = test_hotkey_ordering

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test (test #includes hotkey_config.c directly)
$(TEST_BINARY): $(TEST_SRC)
	@echo "Compiling hotkey ordering test..."
	@if [ ! -f "$(JULIUS_SRC)/core/hotkey_config.c" ]; then \
		echo "Error: Julius source not found at $(JULIUS_SRC)/core/hotkey_config.c"; \
		echo "Set JULIUS_SRC to the Julius src directory"; \
		exit 1; \
	fi
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(LDFLAGS)

# Run tests
# - On FIXED code: Tests pass (correct enum-to-string mapping)
# - On BUGGY code: Tests fail (swapped mapping detected)
test: $(TEST_BINARY)
	@echo "Running hotkey ordering tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Hotkey enum-to-string mapping is incorrect."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
