// In queue.c
#include "queue.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

// Define the struct queue
struct queue {
    int size;
    int front;
    int rear;
    int count;
    void **elements;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
};

// Rest of your implementation...

// In queue.c
bool queue_push(queue_t *q, void *elem) {

    if (q == NULL) {
        return false;
    }

    pthread_mutex_lock(&q->lock);


    while (q->count == q->size) {
        // Queue is full, wait for it to become not full
        pthread_cond_wait(&q->not_full, &q->lock);
    }

    // printf("Producer count before push: %d by thread: %lu\n", q->count, pthread_self());
    q->elements[q->rear] = elem;
    q->rear = (q->rear + 1) % q->size;
    q->count += 1;
    // printf("Producer count after push: %d\n", q->count);

    

    pthread_cond_signal(&q->not_empty);
    // printf("Signaled not_empty in push\n"); // Add this line

    // printf("Producer thread pushed element pointer %p\n", elem);
    // printf("Producer thread pushed element %d\n", *((int*)elem));
    pthread_mutex_unlock(&q->lock);

    return true;
}

// In queue.c
bool queue_pop(queue_t *q, void **elem) {

    if (q == NULL) {
        return false;
    }

    pthread_mutex_lock(&q->lock);

    // printf("Pop thread acquired lock\n");

    while (q->count == 0) {
        // Queue is empty, wait for it to become not empty
        // printf("Pop thread waiting for the queue to become not empty...\n");
        pthread_cond_wait(&q->not_empty, &q->lock);
    }

    *elem = q->elements[q->front];
    q->front = (q->front + 1) % q->size;
    q->count -= 1;

    pthread_cond_signal(&q->not_full);
    // printf("Signaled not_full in pop\n"); // Add this line
    // int *a = *(int **)elem;
    // printf("#################################### Consumer thread popped element %d\n", *a);

    pthread_mutex_unlock(&q->lock);
    // printf("Pop thread released lock\n");

    return true;
}

// In queue.c
queue_t *queue_new(int size) {
    queue_t *q = malloc(sizeof(queue_t));
    if (q == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    q->size = size;
    q->front = q->rear = 0;
    q->elements = malloc(size * sizeof(void *));
    if (q->elements == NULL) {
        // Handle memory allocation failure
        free(q);
        return NULL;
    }

    q->count = 0; // Initialize count to 0
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);

    return q;
}

// In queue.c
void queue_delete(queue_t **q) {
    if (*q == NULL) {
        return;
    }

    free((*q)->elements);
    pthread_mutex_destroy(&(*q)->lock);
    pthread_cond_destroy(&(*q)->not_empty);
    pthread_cond_destroy(&(*q)->not_full);

    free(*q);
    *q = NULL;
}
