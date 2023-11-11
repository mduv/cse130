#include <stdio.h>
#include <pthread.h>
#include "rwlock.h"
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_READERS 3
#define NUM_WRITERS 2

rwlock_t *rwlock;

void *reader_thread(void *arg) {
    int id = *(int *) arg;

    printf("Time %ld Reader %d is requesting lock.\n", time(NULL), id);
    reader_lock(rwlock);

    // Simulate reading
    printf("Time %ld Reader %d got the lock.\n", time(NULL), id);

    // Simulate some work
    sleep(1);

    printf("Time %ld Reader %d is releasing lock.\n", time(NULL), id);
    reader_unlock(rwlock);

    pthread_exit(NULL);
}

void *writer_thread(void *arg) {
    int id = *(int *) arg;

    printf("Time %ld Writer %d is requesting lock.\n", time(NULL), id);
    writer_lock(rwlock);

    // Simulate writing
    printf("Time %ld Writer %d got the lock.\n", time(NULL), id);

    // Simulate some work
    sleep(1);

    printf("Time %ld Writer %d is releasing lock.\n", time(NULL), id);
    writer_unlock(rwlock);

    pthread_exit(NULL);
}

int main() {
    rwlock = rwlock_new(N_WAY, 2); // Set N-WAY priority with n = 2

    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];

    int reader_ids[NUM_READERS];
    int writer_ids[NUM_WRITERS];

    for (int i = 0; i < NUM_READERS; ++i) {
        reader_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader_thread, &reader_ids[i]);
    }

    for (int i = 0; i < NUM_WRITERS; ++i) {
        writer_ids[i] = i + 1;
        pthread_create(&writers[i], NULL, writer_thread, &writer_ids[i]);
    }

    for (int i = 0; i < NUM_READERS; ++i) {
        pthread_join(readers[i], NULL);
    }

    for (int i = 0; i < NUM_WRITERS; ++i) {
        pthread_join(writers[i], NULL);
    }

    rwlock_delete(&rwlock);

    return 0;
}
