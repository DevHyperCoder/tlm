CC=gcc
CFLAGS=-I.

compile: main.o
	$(CC) -o tlm *.c
	rm -f *.o

run:
	./tlm

clean:
	rm -f *.o

.PHONY: clean
