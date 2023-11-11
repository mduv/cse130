#include <pthread.h>
#include <stdlib.h>

typedef enum { READERS, WRITERS, N_WAY } PRIORITY;

typedef struct rwlock {
    pthread_mutex_t g;
    pthread_cond_t cond;
    int num_readers_active;
    int num_writers_waiting;
    int num_readers_waiting;
    int num_readers_since_last_write;
    int writer_active;
    PRIORITY priority;
    int n;
} rwlock_t;

rwlock_t *rwlock_new(PRIORITY p, int n) {
    rwlock_t *rwlock = (rwlock_t *) malloc(sizeof(rwlock_t));
    if (!rwlock) {
        // Handle memory allocation failure
        return NULL;
    }

    pthread_mutex_init(&rwlock->g, NULL);
    pthread_cond_init(&rwlock->cond, NULL);
    rwlock->num_readers_active = 0;
    rwlock->num_writers_waiting = 0;
    rwlock->num_readers_waiting = 0;
    rwlock->num_readers_since_last_write = 0;
    rwlock->writer_active = 0;
    rwlock->priority = p;
    rwlock->n = (p == N_WAY) ? n : 0;

    return rwlock;
}

void rwlock_delete(rwlock_t **rw) {
    if (rw && *rw) {
        pthread_mutex_destroy(&(*rw)->g);
        pthread_cond_destroy(&(*rw)->cond);
        free(*rw);
        *rw = NULL;
    }
}

void reader_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->g);

    rw->num_readers_waiting++;

    while ((rw->priority == WRITERS && (rw->num_writers_waiting > 0)) || (rw->writer_active)
           || ((rw->priority == N_WAY) && (rw->num_writers_waiting > 0)
               && (rw->num_readers_since_last_write >= rw->n))) {
        pthread_cond_wait(&rw->cond, &rw->g);
    }

    rw->num_readers_active++;

    rw->num_readers_since_last_write++;

    rw->num_readers_waiting--;

    pthread_mutex_unlock(&rw->g);
}

void reader_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->g);

    rw->num_readers_active--;

    if (rw->num_readers_active == 0) {
        pthread_cond_broadcast(&rw->cond);
    }

    pthread_mutex_unlock(&rw->g);
}

void writer_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->g);

    rw->num_writers_waiting++;

    while ((rw->priority == READERS && rw->num_readers_waiting > 0) || (rw->num_readers_active > 0)
           || (rw->writer_active)
           || ((rw->priority == N_WAY) && (rw->num_readers_waiting > 0)
               && (rw->num_readers_since_last_write < rw->n))) {
        pthread_cond_wait(&rw->cond, &rw->g);
    }

    rw->num_writers_waiting--;
    rw->writer_active = 1;

    pthread_mutex_unlock(&rw->g);
}

void writer_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->g);

    rw->writer_active = 0;
    rw->num_readers_since_last_write = 0;
    pthread_cond_broadcast(&rw->cond);

    pthread_mutex_unlock(&rw->g);
}
