# Sound Mixing Test Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

# The test file contains both buggy and fixed implementations inline
# to allow direct comparison

test: test_snd_mix.c
	$(CC) $(CFLAGS) -o test_snd_mix test_snd_mix.c
	@echo "Running sound mixing tests:"
	@./test_snd_mix

clean:
	rm -f test_snd_mix

.PHONY: test clean
