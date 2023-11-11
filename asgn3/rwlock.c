#include <pthread.h>
#include <stdlib.h>

#include "rwlock.h"

struct rwlock {
    pthread_mutex_t mutex;
    pthread_cond_t reader_cv;
    pthread_cond_t writer_cv;
    int reader_count;
    int writer_count;
    PRIORITY priority;
    uint32_t n;
};

rwlock_t* rwlock_new(PRIORITY p, uint32_t n) {
    rwlock_t* rwlock = (rwlock_t*)malloc(sizeof(rwlock_t));
    if (!rwlock) {
        // Handle memory allocation failure
        return NULL;
    }

    pthread_mutex_init(&rwlock->mutex, NULL);
    pthread_cond_init(&rwlock->reader_cv, NULL);
    pthread_cond_init(&rwlock->writer_cv, NULL);
    rwlock->reader_count = 0;
    rwlock->writer_count = 0;
    rwlock->priority = p;
    rwlock->n = (p == N_WAY) ? n : 0;

    return rwlock;
}

void rwlock_delete(rwlock_t **rw) {
    if (rw && *rw) {
        pthread_mutex_destroy(&(*rw)->mutex);
        pthread_cond_destroy(&(*rw)->reader_cv);
        pthread_cond_destroy(&(*rw)->writer_cv);
        free(*rw);
        *rw = NULL;
    }
}

void reader_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);

    while ((rw->priority == WRITERS && rw->writer_count > 0) || 
           (rw->priority == N_WAY && rw->writer_count > 0)) {
        pthread_cond_wait(&rw->reader_cv, &rw->mutex);
    }

    rw->reader_count++;

    pthread_mutex_unlock(&rw->mutex);
}

void reader_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);

    rw->reader_count--;

    if (rw->reader_count == 0) {
        pthread_cond_signal(&rw->writer_cv);
    }

    pthread_mutex_unlock(&rw->mutex);
}

void writer_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);

    while ((rw->priority == READERS && rw->reader_count > 0) || 
           (rw->priority == N_WAY && (uint32_t)rw->reader_count >= rw->n)) {
        pthread_cond_wait(&rw->writer_cv, &rw->mutex);
    }

    rw->writer_count++;

    pthread_mutex_unlock(&rw->mutex);
}

void writer_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);

    rw->writer_count--;

    if (rw->priority == N_WAY && rw->writer_count == 0 && rw->reader_count > 0) {
        pthread_cond_broadcast(&rw->reader_cv);
    } else {
        pthread_cond_signal(&rw->writer_cv);
    }

    pthread_mutex_unlock(&rw->mutex);
}
