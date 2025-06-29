CC = gcc
CFLAGS = -Wall -O2 -pthread
TARGET = zad1
SRC = zad1.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< -lm

clean:
	rm -f $(TARGET)