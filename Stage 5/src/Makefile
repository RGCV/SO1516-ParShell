      CC=gcc
  CFLAGS=-g -Wall -Wextra
   PROGS=par-shell par-shell-terminal fibonacci div0
TXTFILES=log.txt par-shell-out-*.txt
  OTHERS=/tmp/par-shell-in /tmp/par-shell-terminal-*

all: par-shell par-shell-terminal fibonacci div0

par-shell: commandlinereader.o pidlist.o list.o wrappers.o par-shell.o
	$(CC) -pthread -o par-shell par-shell.o commandlinereader.o pidlist.o list.o wrappers.o
par-shell-terminal: commandlinereader.o wrappers.o par-shell-terminal.o
	$(CC) -pthread -o par-shell-terminal par-shell-terminal.o commandlinereader.o wrappers.o
fibonacci: fibonacci.c
	$(CC) $(CFLAGS) -o fibonacci fibonacci.c
div0: div.c
	$(CC) $(CFLAGS) -o div0 div.c

par-shell.o: commandlinereader.c pidlist.c list.c wrappers.c par-shell.c
	$(CC) $(CFLAGS) -c par-shell.c
par-shell-terminal.o: commandlinereader.c wrappers.c par-shell-terminal.c
	$(CC) $(CFLAGS) -c par-shell-terminal.c
commandlinereader.o: commandlinereader.h commandlinereader.c
	$(CC) $(CFLAGS) -c commandlinereader.c
list.o: list.h list.c
	$(CC) $(CFLAGS) -c list.c
pidlist.o: pidlist.h pidlist.c
	$(CC) $(CFLAGS) -c pidlist.c
wrappers.o: wrappers.h wrappers.c
	$(CC) $(CFLAGS) -c wrappers.c -pthread

clean:
	$(RM) *.o
	$(RM) $(PROGS)
	$(RM) $(TXTFILES)
	$(RM) $(OTHERS)
