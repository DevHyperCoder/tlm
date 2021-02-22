CC=gcc
CFLAGS=-I.

compile: main.o utils.o
	$(CC) -o tlm *.c -lncurses -lform
	rm -f *.o

run:
	./tlm

clean:
	rm -f *.o

.PHONY: clean
