CC=clang 
CFLAGS=-Wall -Wextra -Werror -pedantic

all: httpserver

httpserver: httpserver.c asgn4_helper_funcs.a
	$(CC) $(CFLAGS) -o httpserver httpserver.c asgn4_helper_funcs.a

clean:
	rm -f httpserver *.o

format:
	clang-format -i -style=file *.[ch]