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
    void **elements;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
};

// Rest of your implementation...
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

// In queue.c
bool queue_push(queue_t *q, void *elem) {
    pthread_mutex_lock(&q->lock);

    while ((q->rear + 1) % q->size == q->front) {
        // Queue is full, wait for it to become not full
        printf("Queue is full. Waiting for it to become not full...\n");  // Add this line
        pthread_cond_wait(&q->not_full, &q->lock);
        printf("Woke up from wait. Checking queue again...\n");  // Add this line
    }

    q->elements[q->rear] = elem;
    q->rear = (q->rear + 1) % q->size;

    pthread_cond_signal(&q->not_empty);
    printf("Signaled not_empty in push\n");  // Add this line
    pthread_mutex_unlock(&q->lock);

    return true;
}


bool queue_pop(queue_t *q, void **elem) {
    pthread_mutex_lock(&q->lock);

    printf("Entering queue_pop\n");

    while (q->front == q->rear) {
        // Queue is empty, wait for it to become not empty
        printf("Queue is empty. Waiting for it to become not empty...\n");
        pthread_cond_wait(&q->not_empty, &q->lock);
    }

    *elem = q->elements[q->front];
    q->front = (q->front + 1) % q->size;

    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->lock);

    printf("Exiting queue_pop\n");

    return true;
}

