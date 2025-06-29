CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = 
TARGETS = zad1 catcher sender

all: $(TARGETS)

zad1: zad1.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

catcher: catcher.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

sender: sender.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean