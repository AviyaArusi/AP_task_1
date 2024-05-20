CC = gcc

.PHONY: all clean

all: myshell

myshell: shell2.c
	$(CC) shell2.c -o myshell


clean:
	rm -f *.o myshell *.so
