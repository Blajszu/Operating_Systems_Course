# Kompilator
CC = gcc
# Flagi kompilacji
CFLAGS = -Wall -Wextra -std=c99

# Lista plików źródłowych (możesz dodać więcej)
SRCS = client.c server.c
# Generuj nazwy plików wykonywalnych na podstawie .c (zad1, zad2)
TARGETS = $(SRCS:.c=)

# Domyślna reguła - kompiluje wszystkie programy
all: $(TARGETS)

# Generuj osobne reguły dla każdego pliku
$(TARGETS): %: %.c
	$(CC) $(CFLAGS) -o $@ $<

# Czyszczenie - usuwa wszystkie pliki wykonywalne
clean:
	rm -f $(TARGETS)

# Informacja, że te cele nie są właściwymi plikami
.PHONY: all clean