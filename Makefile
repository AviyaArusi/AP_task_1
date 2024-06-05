CC = gcc

.PHONY: all clean

all: myshell

myshell: final_shell.o queue.o keyval_table.o
	$(CC) queue.o final_shell.o keyval_table.o -o myshell
queue.o: queue.c queue.h
	$(CC) -c queue.c 
final_shell.o: final_shell.c
	$(CC) -c final_shell.c
keyval_table.o: keyval_table.c keyval_table.h
	$(CC) -c keyval_table.c 

clean:
	rm -f *.o myshell *.so