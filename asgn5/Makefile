CC=clang
CFLAGS=-Wall -Wextra -Werror -pedantic

all: cacher

cacher: cacher.c
	$(CC) $(CFLAGS) -o cacher cacher.c

clean:
	rm -f cacher

format:
	clang-format -i -style=file *.[ch]
