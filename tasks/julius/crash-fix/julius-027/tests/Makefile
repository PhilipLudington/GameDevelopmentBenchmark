CC ?= clang
CFLAGS = -Wall -Wextra -g -I$(JULIUS_SRC) -I$(JULIUS_SRC)/..
JULIUS_SRC ?= .

TEST_SRC = test_population_ratio.c
STUB_SRC = population_stubs.c

TEST_BINARY = test_population_ratio

all: $(TEST_BINARY)

$(TEST_BINARY): $(TEST_SRC) $(STUB_SRC)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(STUB_SRC) $(LDFLAGS)

test: $(TEST_BINARY)
	@echo "Running population ratio test..."
	@./$(TEST_BINARY); \
	exit_code=$$?; \
	if [ $$exit_code -eq 0 ]; then \
		echo "Test completed successfully."; \
	else \
		echo "Test FAILED with exit code $$exit_code"; \
	fi; \
	exit $$exit_code

clean:
	rm -f $(TEST_BINARY)

.PHONY: all test clean
