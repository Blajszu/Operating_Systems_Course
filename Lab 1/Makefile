CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -g -O0

all: countdown

countdown: countdown.c
	$(CC) $(CFLAGS) -o countdown countdown.c

clean:
	rm -f countdown

.PHONY: all countdown clean
