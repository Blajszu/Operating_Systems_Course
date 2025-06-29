CC = gcc
CFLAGS = -Wall -Wextra -g -O0

SRC = flipper.c
EXEC = flipperd1

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC)

clean:
	rm -f $(EXEC) $(EXEC) flipper *.txt zad1 zad2

PHONY: all clean