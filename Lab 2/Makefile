CC = gcc
CFLAGS = -Wall -Wextra -DMAX_ITER=1000 -g -O0


all: client_dynamic client_shared client_static

libcollatz.a: collatz.o
	ar rcs libcollatz.a collatz.o

libcollatz.so: collatz_shared.o
	$(CC) -shared -Wl,-soname,libcollatz.so.1 -o libcollatz.so.1.0.1 collatz_shared.o -lc
	ln -sf libcollatz.so.1.0.1 libcollatz.so.1
	ln -sf libcollatz.so.1 libcollatz.so

collatz.o: collatz.c collatz.h
	$(CC) $(CFLAGS) -c collatz.c -o collatz.o

collatz_shared.o: collatz.c collatz.h
	$(CC) $(CFLAGS) -fPIC -c collatz.c -o collatz_shared.o

client.o: client.c
	$(CC) $(CFLAGS) -c client.c -o client.o

client_static: client.o libcollatz.a
	$(CC) $(CFLAGS) client.o libcollatz.a -o client_static

client_shared: client.o libcollatz.so
	$(CC) $(CFLAGS) client.o -lcollatz -L./ -Wl,-rpath,. -o client_shared

client_dynamic: client.o libcollatz.so
	$(CC) $(CFLAGS) -DDYNAMIC_LOAD -o client_dynamic client.c -ldl

clean:
	rm -f *.o *.a *.so* client_static client_shared client_dynamic

.PHONY: all clean client_static client_shared client_dynamic