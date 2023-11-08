// In test_queue.c
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Create a queue with a maximum size of 5
    queue_t *q = queue_new(5);
    if (q == NULL) {
        fprintf(stderr, "Failed to create the queue\n");
        return 1;
    }

    // Test pushing and popping elements
    for (int i = 1; i <= 7; ++i) {
        int *elem = malloc(sizeof(int));
        *elem = i;
        // printf("here\n");
        // queue_push(q, elem);
        // printf("Pushed element %d to the queue\n", i);

        if (!queue_push(q, elem)) {
            printf("Failed to push element %d to the queue\n", i);
        } else {
            printf("Pushed element %d to the queue\n", i);
        }
    }

    // In test_queue.c or wherever you consume elements
    for (int i = 1; i <= 7; ++i) {
        int *elem;
        while (!queue_pop(q, (void **) &elem)) {
            // Queue is empty, wait for it to become not empty
            printf("Queue is empty. Waiting for it to become not empty...\n");
            // Add a delay or sleep here to avoid busy-waiting
        }
        printf("Popped element %d from the queue\n", *elem);
        free(elem);
    }

    // Clean up
    queue_delete(&q);

    return 0;
}
