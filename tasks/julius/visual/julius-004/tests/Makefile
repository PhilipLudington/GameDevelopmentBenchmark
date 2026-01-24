# Makefile for julius-004 tooltip trailing newline test
#
# This test uses static analysis to verify that the text layout algorithm
# checks the width boundary BEFORE adding word_width, not AFTER.
#
# The bug: In text_draw_multiline() and text_measure_multiline(), the code
# adds word_width to current_width first, then checks if current_width >= box_width.
# This causes incorrect line breaks and trailing newlines in tooltips.
#
# Test validation:
# - FIXED code: checks "current_width + word_width >= box_width" → test passes
# - BUGGY code: checks "current_width >= box_width" after adding → test fails

CC ?= clang
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Julius source directory (set by test runner)
JULIUS_SRC ?= .

# Test targets
TEST_BINARY = test_tooltip_newline

.PHONY: all clean test

all: $(TEST_BINARY)

# Build test (standalone, does static analysis of text.c)
$(TEST_BINARY): test_tooltip_newline.c
	@echo "Compiling tooltip newline static analysis test..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Run tests
# The test reads graphics/text.c and checks the width checking pattern
test: $(TEST_BINARY)
	@echo "Running tooltip newline tests..."
	@echo ""
	@if [ ! -f "$(JULIUS_SRC)/graphics/text.c" ]; then \
		echo "Error: Julius source not found at $(JULIUS_SRC)/graphics/text.c"; \
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
		echo "Width checked AFTER adding word_width (trailing newline bug)."; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)
	rm -rf *.dSYM
