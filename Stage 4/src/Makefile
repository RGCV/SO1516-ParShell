   CC=gcc
CFLAGS=-g -Wall -Wextra
all: par-shell fibonacci div0

par-shell: commandlinereader.o list.o par-shell.o
	$(CC) -pthread -o par-shell par-shell.o commandlinereader.o list.o
div0: div.c
	$(CC) $(CFLAGS) -o div0 div.c
fibonacci: fibonacci.c
	$(CC) $(CFLAGS) -o fibonacci fibonacci.c

par-shell.o: commandlinereader.c list.c par-shell.c
	$(CC) $(CFLAGS) -c par-shell.c
commandlinereader.o: commandlinereader.h commandlinereader.c
	$(CC) $(CFLAGS) -c commandlinereader.c
list.o: list.h list.c
	$(CC) $(CFLAGS) -c list.c

clean:
	$(RM) *.o par-shell fibonacci div0
