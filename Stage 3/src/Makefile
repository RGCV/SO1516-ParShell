    CC=gcc
CFLAGS=-g -Wall -Wextra

all: par-shell fibonacci
par-shell: par-shell.o commandlinereader.o list.o
	$(CC) -pthread -o par-shell par-shell.o commandlinereader.o list.o
fibonacci: fibonacci.c
	$(CC) $(CFLAGS) -o fibonacci fibonacci.c

par-shell.o: commandlinereader.c par-shell.c list.c
	$(CC) $(CFLAGS) -c par-shell.c
commandlinereader.o: commandlinereader.h commandlinereader.c
	$(CC) $(CFLAGS) -c commandlinereader.c
list.o: list.h list.c
	$(CC) $(CFLAGS) -c list.c

clean:
	$(RM) *.o par-shell fibonacci
