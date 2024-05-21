CC = gcc

.PHONY: all clean

all: myshell

myshell: shell2.o queue.o
	$(CC) queue.o shell2.o  -o myshell
queue.o: queue.c queue.h
	$(CC) -c queue.c 
shell2.o: shell2.c
	$(CC) -c shell2.c


clean:
	rm -f *.o myshell *.so
