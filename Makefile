CC=gcc
CFLAGS=-I.

compile: main.o
	$(CC) -o tlm *.c -lncurses
	rm -f *.o

run:
	./tlm

clean:
	rm -f *.o

.PHONY: clean
