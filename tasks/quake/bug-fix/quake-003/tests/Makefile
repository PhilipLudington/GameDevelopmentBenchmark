# Network Prediction Test Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -lm

# The test file contains both buggy and fixed implementations inline
# to allow direct comparison

test: test_prediction.c
	$(CC) $(CFLAGS) -o test_prediction test_prediction.c
	@echo "Running network prediction tests:"
	@./test_prediction

clean:
	rm -f test_prediction

.PHONY: test clean
