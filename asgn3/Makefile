CC=clang
CFLAGS=-Wall -Wextra -Werror -pedantic -pthread -Wunused-parameter

all: queue.o rwlock.o test_queue # test_rwlock
#rwlock.o

queue.o: queue.c
	$(CC) $(CFLAGS) -c $^ -o $@

rwlock.o: rwlock.c
	$(CC) $(CFLAGS) -c $^ -o $@

test_queue: test_queue.c queue.o
	$(CC) $(CFLAGS) $^ -o $@

# test_rwlock: test_rwlock.c rwlock.o
# 	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o

format:
	clang-format -i -style=file *.[ch]
