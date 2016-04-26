    CC=gcc
CFLAGS=-g -Wall -Wextra

all: par-shell fibonacci
par-shell: par-shell.o commandlinereader.o
	$(CC) -o par-shell par-shell.o commandlinereader.o
fibonacci: fibonacci.c
	$(CC) -o fibonacci fibonacci.c 

par-shell.o: commandlinereader.c par-shell.c
	$(CC) $(CFLAGS) -c par-shell.c
commandlinereader.o: commandlinereader.h commandlinereader.c
	$(CC) $(CFLAGS) -c commandlinereader.c

clean:
	$(RM) *.o par-shell fibonacci
