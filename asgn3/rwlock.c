// In rwlock.c
#include "rwlock.h"
#include <pthread.h>
#include <stdlib.h>

struct rwlock {
    pthread_mutex_t lock;
    pthread_cond_t readers_cv;
    pthread_cond_t writers_cv;
    int readers;
    int writers;
    PRIORITY priority;
    int n;
};

rwlock_t *rwlock_new(PRIORITY p, uint32_t n) {
    rwlock_t *rw = malloc(sizeof(rwlock_t));
    if (rw == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    pthread_mutex_init(&rw->lock, NULL);
    pthread_cond_init(&rw->readers_cv, NULL);
    pthread_cond_init(&rw->writers_cv, NULL);
    rw->readers = 0;
    rw->writers = 0;
    rw->priority = p;
    rw->n = n;

    return rw;
}

void rwlock_delete(rwlock_t **rw) {
    if (*rw == NULL) {
        return;
    }

    pthread_mutex_destroy(&(*rw)->lock);
    pthread_cond_destroy(&(*rw)->readers_cv);
    pthread_cond_destroy(&(*rw)->writers_cv);

    free(*rw);
    *rw = NULL;
}

void reader_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);

    while ((rw->priority == READERS && rw->writers > 0)
           || (rw->priority == WRITERS && (rw->readers > 0 || rw->writers > 0))
           || (rw->priority == N_WAY && rw->writers > 0 && rw->readers < rw->n)) {
        pthread_cond_wait(&rw->readers_cv, &rw->lock);
    }

    rw->readers++;

    pthread_mutex_unlock(&rw->lock);
}

void reader_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);

    rw->readers--;

    if (rw->readers == 0 && rw->writers > 0) {
        pthread_cond_signal(&rw->writers_cv);
    }

    pthread_mutex_unlock(&rw->lock);
}

void writer_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);

    while ((rw->priority == WRITERS && (rw->readers > 0 || rw->writers > 0))
           || (rw->priority == N_WAY && (rw->readers > 0 || rw->writers > 0))) {
        pthread_cond_wait(&rw->writers_cv, &rw->lock);
    }

    rw->writers++;

    pthread_mutex_unlock(&rw->lock);
}

void writer_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);

    rw->writers--;

    if (rw->priority == N_WAY && rw->readers < rw->n) {
        pthread_cond_signal(&rw->readers_cv);
    } else {
        pthread_cond_broadcast(&rw->writers_cv);
    }

    pthread_mutex_unlock(&rw->lock);
}
