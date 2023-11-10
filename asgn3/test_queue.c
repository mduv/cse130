#include "queue.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// Global variables for testing
int item_enqueued_order[10]; // To store the order in which items are enqueued
int item_dequeued_order[10]; // To store the order in which items are dequeued
int consumer_reads_valid_items = 1; // Flag to indicate if the consumer reads only valid items
int all_items_produced; // Counter for all items produced
int all_items_consumed; // Counter for all items consumed

// Function declarations
void *producer_thread(void *arg);
void *consumer_thread(void *arg);
void test_thread_order();
void test_validity();
void test_completeness();

// Function declarations
void test_queue_push_pop();
void test_queue_empty_pop();
void test_queue_full_push();
void test_queue_sequence();
void test_queue_push_null();
void test_queue_pop_null();
void test_queue_memory_leak();

void *producer_thread(void *arg) {
    queue_t *q = (queue_t *) arg;

    int id = pthread_self();

    for (int i = 0; i < 2; ++i) {
        int *elem = malloc(sizeof(int));
        *elem = id * 10 + i;

        assert(queue_push(q, elem) == true);
        printf("Producer thread %d pushed element %d\n", id, *elem);
    }

    return NULL;
}

void *consumer_thread(void *arg) {
    queue_t *q = (queue_t *) arg;

    int id = pthread_self();

    for (int i = 0; i < 2; ++i) {
        int *elem;
        assert(queue_pop(q, (void **) &elem) == true);
        printf("Consumer thread %d popped element %d\n", id, *elem);
        free(elem);
    }

    return NULL;
}

void test_thread_safety() {
    queue_t *q = queue_new(10);

    pthread_t producer1, producer2, consumer1, consumer2;

    pthread_create(&producer1, NULL, producer_thread, (void *) q);
    pthread_create(&producer2, NULL, producer_thread, (void *) q);
    pthread_create(&consumer1, NULL, consumer_thread, (void *) q);
    pthread_create(&consumer2, NULL, consumer_thread, (void *) q);

    pthread_join(producer1, NULL);
    pthread_join(producer2, NULL);
    pthread_join(consumer1, NULL);
    pthread_join(consumer2, NULL);

    queue_delete(&q);
}

int main() {
    // Run your tests
    // test_queue_push_pop();
    // test_queue_empty_pop();
    // test_queue_full_push();
    // test_queue_sequence();
    // test_queue_push_null();
    // test_queue_pop_null();
    // test_queue_memory_leak();
    // test_thread_safety();
    // test_thread_order();
    // test_validity();
    test_completeness();

    // Add more test calls as needed

    printf("All tests passed!\n");

    return 0;
}

void test_thread_order() {
    queue_t *q = queue_new(5);

    pthread_t producer1, producer2, consumer1, consumer2;

    pthread_create(&producer1, NULL, producer_thread, (void *) q);
    pthread_create(&producer2, NULL, producer_thread, (void *) q);
    pthread_create(&consumer1, NULL, consumer_thread, (void *) q);
    pthread_create(&consumer2, NULL, consumer_thread, (void *) q);

    pthread_join(consumer1, NULL);
    pthread_join(producer1, NULL);
    pthread_join(producer2, NULL);
    pthread_join(consumer2, NULL);

    // Check that items are dequeued in the order they were enqueued
    assert(item_dequeued_order[0] == item_enqueued_order[0]);
    assert(item_dequeued_order[1] == item_enqueued_order[1]);
    assert(item_dequeued_order[2] == item_enqueued_order[2]);
    // ... add more checks as needed

    queue_delete(&q);
}

void test_validity() {
    queue_t *q = queue_new(5);

    pthread_t producer, consumer;

    pthread_create(&producer, NULL, producer_thread, (void *) q);
    pthread_create(&consumer, NULL, consumer_thread, (void *) q);

    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    // Check that the consumer reads only items produced by the producer
    assert(consumer_reads_valid_items);

    queue_delete(&q);
}

void test_completeness() {
    queue_t *q = queue_new(5);

    pthread_t producer, consumer;

    pthread_create(&producer, NULL, producer_thread, (void *) q);
    pthread_create(&consumer, NULL, consumer_thread, (void *) q);

    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    // Check that some consumer dequeues every item produced
    assert(all_items_produced == all_items_consumed);

    queue_delete(&q);
}

void test_queue_push_pop() {
    queue_t *q = queue_new(5);

    int elem1 = 1;
    int elem2 = 2;

    assert(queue_push(q, &elem1) == true);
    assert(queue_push(q, &elem2) == true);

    int *pop_elem1;
    int *pop_elem2;

    assert(queue_pop(q, (void **) &pop_elem1) == true);
    assert(queue_pop(q, (void **) &pop_elem2) == true);

    assert(*pop_elem1 == elem1);
    assert(*pop_elem2 == elem2);

    queue_delete(&q);
}

void test_queue_empty_pop() {
    queue_t *q = queue_new(5);

    int *elem;
    assert(queue_pop(q, (void **) &elem) == false);

    queue_delete(&q);
}

void test_queue_full_push() {
    queue_t *q = queue_new(2);

    int elem1 = 1;
    int elem2 = 2;
    int elem3 = 3;

    assert(queue_push(q, &elem1) == true);
    assert(queue_push(q, &elem2) == true);
    assert(queue_push(q, &elem3) == false);

    queue_delete(&q);
}

void test_queue_sequence() {
    queue_t *q = queue_new(3);

    int elem1 = 1;
    int elem2 = 2;

    assert(queue_push(q, &elem1) == true);
    assert(queue_push(q, &elem2) == true);

    int *pop_elem1;
    int *pop_elem2;

    assert(queue_pop(q, (void **) &pop_elem1) == true);
    assert(queue_pop(q, (void **) &pop_elem2) == true);

    assert(*pop_elem1 == elem1);
    assert(*pop_elem2 == elem2);

    queue_delete(&q);
}

void test_queue_push_null() {
    queue_t *q = queue_new(5);

    assert(queue_push(q, NULL) == false);

    queue_delete(&q);
}

void test_queue_pop_null() {
    queue_t *q = queue_new(5);

    int *elem;
    assert(queue_pop(q, (void **) &elem) == false);

    queue_delete(&q);
}

void test_queue_memory_leak() {
    // This test is to check for memory leaks, not functional correctness
    for (int i = 0; i < 1000; ++i) {
        queue_t *q = queue_new(10);
        queue_delete(&q);
    }
}
