# Kompilator
CC = gcc
# Flagi kompilacji
CFLAGS = -Wall -Wextra -std=gnu11 -pthread
# Flagi debugowania
DEBUG_FLAGS = -g -DDEBUG

# Pliki źródłowe
SERVER_SRC = server.c
CLIENT_SRC = client.c
# Pliki wykonywalne
SERVER_EXE = server
CLIENT_EXE = client

.PHONY: all clean debug

all: $(SERVER_EXE) $(CLIENT_EXE)

# Kompilacja serwera
$(SERVER_EXE): $(SERVER_SRC)
	$(CC) $(CFLAGS) $< -o $@

# Kompilacja klienta
$(CLIENT_EXE): $(CLIENT_SRC)
	$(CC) $(CFLAGS) $< -o $@

# Wersja debug
debug: CFLAGS += $(DEBUG_FLAGS)
debug: all

# Czyszczenie
clean:
	rm -f $(SERVER_EXE) $(CLIENT_EXE) *.o

# Uruchomienie serwera (przykład)
run_server: $(SERVER_EXE)
	./$(SERVER_EXE) 8080

# Uruchomienie klienta (przykład)
run_client: $(CLIENT_EXE)
	./$(CLIENT_EXE) client1 127.0.0.1 8080