CC=clang
CFLAGS=-Wall -Wextra -Werror -pedantic

all: queue.o test_queue
#rwlock.o

queue.o: queue.c
	$(CC) $(CFLAGS) -c $^ -o $@

# rwlock.o: rwlock.c
# 	$(CC) $(CFLAGS) -c $^ -o $@

test_queue: test_queue.c queue.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o
