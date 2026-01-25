CC ?= clang
CFLAGS = -Wall -Wextra -g -I$(JULIUS_SRC) -I$(JULIUS_SRC)/..
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer
JULIUS_SRC ?= .

TEST_SRC = test_scenario_leak.c
STUB_SRC = scenario_stubs.c

TEST_BINARY = test_scenario_leak
TEST_BINARY_ASAN = test_scenario_leak_asan

all: $(TEST_BINARY_ASAN)

$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

$(TEST_BINARY_ASAN): $(TEST_SRC) $(STUB_SRC)
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

test: $(TEST_BINARY_ASAN)
	@echo "Running scenario memory leak test with ASan..."
	@ASAN_OPTIONS="detect_leaks=0:abort_on_error=1:print_stacktrace=1" ./$(TEST_BINARY_ASAN); \
	exit_code=$$?; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED with exit code $$exit_code"; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY) $(TEST_BINARY_ASAN)

.PHONY: all test clean
