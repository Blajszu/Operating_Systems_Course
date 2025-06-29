CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pedantic
LDFLAGS = -lm -lrt

.PHONY: all clean

all: zad1 zad2_1 zad2_2

zad1: zad1.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

zad2_1: zad2_1.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

zad2_2: zad2_2.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f zad1 zad2_1 zad2_2 result to_calculate