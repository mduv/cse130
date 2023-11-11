#include "queue.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
int ops = 0;
queue_t *q = NULL;

void pop_check(queue_t *q, void *elem) {
  bool rtn = queue_pop(q, elem);
  if (!rtn) {
    fprintf(stderr, "queue_pop failed!\n");
    exit(2);
  }
}

void push_check(queue_t *q, void *elem) {
  bool rtn = queue_push(q, elem);
  if (!rtn) {
    fprintf(stderr, "queue_push failed!\n");
    exit(3);
  }
}

void *producer_thread(void *arg) {
  // int id = pthread_self();
  int x = *((int *)arg);

  for (int i = 0; i < 150; ++i) {
    int *elem = malloc(sizeof(int));
    *elem = x + i;
    push_check(q, elem);
  }

  return NULL;
}

void *consumer_thread() {

//   for (int i = 0; i < 5; ++i) {
//     int *elem;
//     pop_check(q, (void **)&elem);
//     printf("###### Consumer thread popped element %d\n", *elem);
//     free(elem);
//   }

//   return NULL;
  // int id = pthread_self();
  int last_base_100 = 0;
  int last_base_10 = 0;

  for (int i = 0; i < 300; ++i) {
    int *elem;
    pop_check(q, (void **)&elem);
    // printf("###### Consumer thread popped element %d\n", *elem);

    int value = *elem;
    if (value >= 150) {
      if (value < last_base_100) {
        exit(2);
      } else {
        last_base_100 = value;
      }
    } else {
      if (value < last_base_10) {
        exit(3);
      } else {
        last_base_10 = value;
      }
    }

    free(elem);
  }

  return NULL;
}


void test_q_is_null() {
    // Test for q is NULL
    printf("Running test_q_is_null...\n");
    if (!queue_push(NULL, NULL)) {
        printf("Test working\n");
    } else {
        printf("Test not working\n");
    }
}

void test_push_and_pop_null_element() {
    // Test for pushing a NULL element
    q = queue_new(10);

    printf("Running test_push_null_element...\n");

    if (!queue_push(q, NULL)) {
        printf("Failed pushing NULL\n");
    } else {
        printf("Passed pushing NULL\n");
    }

    void *elem;
    if (!queue_pop(q, &elem)) {
        printf("Failed popping NULL\n");
    } else {
        printf("Passed popping NULL\n");

        // Check if the popped element is NULL
        if (elem == NULL) {
            printf("Popped element is NULL\n");
        } else {
            printf("Popped element is not NULL\n");
        }
    }

    queue_delete(&q);
}

void test_q_size_smaller() {
    // Test for q size is smaller than production or consumption
    printf("Running test_q_size_smaller...\n");

    pthread_t producer, consumer;

    q = queue_new(2); // Set the queue size to 1

    int p = 0;
    pthread_create(&producer, NULL, producer_thread, (void *)&p);
    pthread_create(&consumer, NULL, consumer_thread, NULL);

    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
}


void test_thread_order_correct() {
    // Test for test order within each thread is correct
    printf("Running test_thread_order_correct...\n");

    pthread_t producer1, consumer, producer2;

    int p1 = 0; 
    int p2 = 150;
    pthread_create(&producer1, NULL, producer_thread, (void *)&p1);
    pthread_create(&consumer, NULL, consumer_thread, NULL);
    pthread_create(&producer2, NULL, producer_thread, (void *)&p2);

    pthread_join(producer1, NULL);
    pthread_join(consumer, NULL);
    pthread_join(producer2, NULL);

    
}

int main() {
    q = queue_new(50);
    // test_q_is_null();
    // test_push_and_pop_null_element();
    // test_q_size_smaller();
    test_thread_order_correct();
    queue_delete(&q);
    return 1;
}













// #include "queue.h"

// #include <stdbool.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>
// #include <unistd.h>

// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t c = PTHREAD_COND_INITIALIZER;
// int ops = 0;
// queue_t *q = NULL;

// void pop_check(queue_t *q, void *elem) {
//   bool rtn = queue_pop(q, elem);
//   if (!rtn) {
//     fprintf(stderr, "queue_pop failed!\n");
//     exit(2);
//   }
// }

// void push_check(queue_t *q, void *elem) {
//   bool rtn = queue_push(q, elem);
//   if (!rtn) {
//     fprintf(stderr, "queue_push failed!\n");
//     exit(3);
//   }
// }

// void *producer_thread(void *arg) {
//   // int id = pthread_self();
//   int x = *((int *)arg);

//   for (int i = 0; i < 15; ++i) {
//     int *elem = malloc(sizeof(int));
//     *elem = x + i;
//     push_check(q, elem);
//   }

//   return NULL;



//   // // Push NULL elements
//   // // for (int i = 0; i < 5; ++i) {
//   // //   push_check(q, NULL);
//   // // }


//   // return NULL;
// }

// void *consumer_thread() {

//   // for (int i = 0; i < 5; ++i) {
//   //   int *elem;
    
//   //   pop_check(q, (void **)&elem);

    
    
//   //   if (elem != NULL) {
//   //     printf("Consumer thread popped element %d\n", *elem);
//   //     free(elem);
//   //   } else {
//   //     printf("Consumer thread popped NULL element\n");
//   //   }
//   // }

//   // return NULL;
//   // int id = pthread_self();
//   int last_base_100;
//   int last_base_10;

//   for (int i = 0; i < 30; ++i) {
//     int *elem;
//     pop_check(q, (void **)&elem);
//     // printf("###### Consumer thread popped element %d\n", *elem);

//     int value = *elem;
//     if (value >= 100) {
//       if (value < last_base_100) {
//         exit(2);
//       } else {
//         last_base_100 = value;
//       }
//     } else {
//       if (value < last_base_10) {
//         exit(3);
//       } else {
//         last_base_10 = value;
//       }
//     }

//     free(elem);
//   }

//   return NULL;
// }

// int main() {

//   q = queue_new(10);
//   if (q == NULL) {
//     return 10;
//   }
//   pthread_t producer1, consumer, producer2; 


//   int p1 = 0; 
//   int p2 = 100;
//   pthread_create(&producer1, NULL, producer_thread, (void *)&p1);
//   pthread_create(&consumer, NULL, consumer_thread, NULL);
//   pthread_create(&producer2, NULL, producer_thread, (void *)&p2);

  
//   pthread_join(producer1, NULL);
//   pthread_join(consumer, NULL);
//   pthread_join(producer2, NULL);


//   return 1;
// }



