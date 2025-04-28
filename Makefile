#CFLAGS = -g -Wall -Wextra -pedantic -Wno-unused-parameter
CFLAGS = -g -Wall -Wextra -pedantic -Wno-unused-parameter
CC = gcc

PROGRAMS = main

.PHONY:	all clean

all: $(PROGRAMS)

Hachage.o: Hachage.c
	gcc $(CFLAGS) -c Hachage.c

Memoire.o: Memoire.c
	gcc $(CFLAGS) -c Memoire.c
	
Parser.o: Parser.c 
	gcc $(CFLAGS) -c Parser.c

CPU.o: CPU.c 
	gcc $(CFLAGS) -c CPU.c
	
main.o: main.c
	$(CC) $(CFLAGS) -c main.c

main: main.o Parser.o Memoire.o Hachage.o CPU.o
	$(CC) -o $@ $(CFLAGS) $^
	
clean:
	rm -f *.o *~ $(PROGRAMS)