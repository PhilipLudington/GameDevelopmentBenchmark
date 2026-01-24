# Makefile for julius-006 clone building validation test
#
# Compiles test that verifies scenario_building_allowed() checks individual
# building types, not just menu groups.
#
# The test #includes the actual scenario/building.c to test the real function.
#
# Test validation:
# - FIXED code: Tests pass (individual building types check their menu restriction)
# - BUGGY code: Tests fail (individual types fall through to default, returning 1)

CC ?= clang
CFLAGS = -Wall -Wextra -g -I$(JULIUS_SRC) -I$(JULIUS_SRC)/..
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Source files - test #includes scenario/building.c directly
TEST_SRC = test_clone_building.c

# Test targets
TEST_BINARY = test_clone_building

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test (test #includes building.c directly)
$(TEST_BINARY): $(TEST_SRC)
	@echo "Compiling clone building validation test..."
	@if [ ! -f "$(JULIUS_SRC)/scenario/building.c" ]; then \
		echo "Error: Julius source not found at $(JULIUS_SRC)/scenario/building.c"; \
		echo "Set JULIUS_SRC to the Julius src directory"; \
		exit 1; \
	fi
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(LDFLAGS)

# Run tests
# - On FIXED code: Tests pass (individual building types properly restricted)
# - On BUGGY code: Tests fail (building types bypass restrictions)
test: $(TEST_BINARY)
	@echo "Running clone building validation tests..."
	@echo ""
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	echo ""; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED (exit code $$exit_code)"; \
		echo "Individual building types are not respecting menu restrictions."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
