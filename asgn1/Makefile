# Makefile influnced by my previous Makefile of Asgn1 in cse13s Fall 2021 with Darrell Long

CC = clang
CFLAGS = -Wall -Wpedantic -Werror -Wextra

all: memory

memory: memory.o
	$(CC) -o memory memory.o

memory.o: memory.c
	$(CC) $(CFLAGS) -c memory.c

clean:
	rm -f memory memory.o

format:
	clang-format -i -style=file *.[ch]
