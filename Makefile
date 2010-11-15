CC = gcc
CFLAGS = -O3

fastafgrep: fastafgrep.c
	$(CC) $(CFLAGS) -D_GNU_SOURCE -Wall -ansi -o fastafgrep fastafgrep.c

clean:
	rm -f fastafgrep
