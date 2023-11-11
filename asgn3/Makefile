CC=clang
CFLAGS=-Wall -Wextra -Werror -pedantic 

all: queue.o rwlock.o 

queue.o: queue.c
	$(CC) $(CFLAGS) -c $^ -o $@

rwlock.o: rwlock.c
	$(CC) $(CFLAGS) -c $^ -o $@

# better_lock_unlock: better_lock_unlock.c rwlock.o
# 	$(CC) $(CFLAGS) -c $^ -o $@

# test_queue: test_queue.c queue.o
# 	$(CC) $(CFLAGS) $^ -o $@

# test_rwlock: test_rwlock.c rwlock.o 
# 	$(CC) $(CFLAGS) $^ -o $@

# test_simple: test_simple.c rwlock.o 
# 	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o

format:
	clang-format -i -style=file *.[ch]
