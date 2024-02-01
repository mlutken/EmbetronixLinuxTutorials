CC = gcc
CFLAGS = -Wall -Wextra

all: test

test: test.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f test

.PHONY: all clean
