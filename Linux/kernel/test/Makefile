CC = gcc
CFLAGS = -Wall -Wextra

all: test select

test: test.c
	$(CC) $(CFLAGS) -o $@ $^

select: select.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f test
	rm -f select

.PHONY: all clean
