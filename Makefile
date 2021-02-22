CC=gcc
CFLAGS=-I.

compile: main.o utils.o
	$(CC) -o tlm *.c -lncurses -lform -lpam
	rm -f *.o

run:
	./tlm

clean:
	rm -f *.o *.orig tlm

.PHONY: clean
