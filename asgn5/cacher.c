#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string.h>

// Define the node structure for the cache
typedef struct Node {
    char* item;             // Item stored in the node
    struct Node* next;      // Pointer to the next node
    struct Node* prev;      // Pointer to the previous node
    time_t timestamp;       // For LRU
    int referencedBit;      // For Clock
} Node;

// Define the cache structure
typedef struct {
    Node* head;             // Pointer to the head of the linked list
    Node* tail;             // Pointer to the tail of the linked list
    int size;               // Maximum size of the cache
    int count;              // Current number of items in the cache
    int compulsoryMisses;
    int capacityMisses;
} Cache;

// Function to initialize an empty cache
void initializeCache(Cache* cache, int size) {
    cache->head = NULL;
    cache->tail = NULL;
    cache->size = size;
    cache->count = 0;
}

// Function to free the memory used by the cache
void freeCache(Cache* cache) {
    Node* current = cache->head;
    Node* next;

    // Traverse the cache and free each node
    while (current != NULL) {
        next = current->next;
        free(current->item);
        free(current);
        current = next;
    }

    // Reset the cache structure
    cache->head = NULL;
    cache->tail = NULL;
    cache->count = 0;
    cache->size = 0;
}


// Function to check if an item is in the cache
int isInCache(Cache* cache, const char* item) {
    Node* current = cache->head;
    while (current != NULL) {
        if (strcmp(current->item, item) == 0) {
            // Item found in the cache
            return 1;
        }
        current = current->next;
    }
    // Item not found in the cache
    return 0;
}

// Function to evict the oldest item (FIFO)
void evictOldest(Cache* cache) {
    if (cache->head != NULL) {
        // Remove the head of the cache (oldest item)
        Node* temp = cache->head;
        cache->head = cache->head->next;

        // Free the memory of the evicted item
        free(temp->item);
        free(temp);

        // Update the previous pointer of the new head
        if (cache->head != NULL) {
            cache->head->prev = NULL;
        } else {
            // If the cache is now empty, update the tail
            cache->tail = NULL;
        }
    }
}

// Function to evict the least recently used item (LRU)
void evictLRU(Cache* cache) {
    if (cache->tail != NULL) {
        // Remove the tail of the cache (least recently used item)
        Node* temp = cache->tail;
        cache->tail = cache->tail->prev;

        // Free the memory of the evicted item
        free(temp->item);
        free(temp);

        // Update the next pointer of the new tail
        if (cache->tail != NULL) {
            cache->tail->next = NULL;
        } else {
            // If the cache is now empty, update the head
            cache->head = NULL;
        }
    }
}

// Function to evict an item using the Clock algorithm
// Helper function to evict an item pointed by the clock hand
void evictClockItem(Cache* cache, Node* clockHand) {
    if (clockHand->prev != NULL) {
        // Update the next pointer of the previous node
        clockHand->prev->next = clockHand->next;
    } else {
        // Clock hand is at the head, update the head
        cache->head = clockHand->next;
    }

    if (clockHand->next != NULL) {
        // Update the previous pointer of the next node
        clockHand->next->prev = clockHand->prev;
    } else {
        // Clock hand is at the tail, update the tail
        cache->tail = clockHand->prev;
    }

    // Free the memory of the evicted item
    free(clockHand->item);
    free(clockHand);
}

// Function to evict an item using the Clock algorithm
void evictClock(Cache* cache) {
    if (cache->head == NULL) {
        // Cache is empty, nothing to evict
        return;
    }

    Node* clockHand = cache->head;
    while (1) {
        if (clockHand->referencedBit == 0) {
            // Found an unreferenced item, evict it
            evictClockItem(cache, clockHand);
            break;
        } else {
            // Set referenced bit to 0 and move the clock hand to the next item
            clockHand->referencedBit = 0;
            clockHand = clockHand->next;

            // If the clock hand reaches the end, wrap around to the head
            if (clockHand == NULL) {
                clockHand = cache->head;
            }
        }
    }
}



// Function to evict the oldest item if the cache is full
void evictIfNeeded(Cache* cache, int evictionPolicy) {
    if (cache->count > cache->size) {
        // Cache is full, eviction needed

        switch (evictionPolicy) {
            case 1:  // FIFO
                evictOldest(cache);
                break;
            case 2:  // LRU
                evictLRU(cache);
                break;
            case 3:  // Clock
                evictClock(cache);
                break;
            default:
                fprintf(stderr, "Invalid eviction policy\n");
                exit(EXIT_FAILURE);
        }

        // Decrement the count of items in the cache
        cache->count--;
    }
}


// Function to add an item to the cache
void addToCache(Cache* cache, const char* item, int evictionPolicy) {
    // Allocate memory for the new node
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the item and copy it
    newNode->item = (char*)malloc(strlen(item) + 1);
    if (newNode->item == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(newNode);
        exit(EXIT_FAILURE);
    }
    strcpy(newNode->item, item);

    // Set default values for the new node
    newNode->next = NULL;
    newNode->prev = NULL;
    newNode->timestamp = time(NULL);  // For LRU
    newNode->referencedBit = 0;  // For Clock

    // Add the new node to the cache based on the eviction policy
    switch (evictionPolicy) {
        case 1:  // FIFO
            // Add the new node to the end of the cache
            if (cache->tail == NULL) {
                // Cache is empty
                cache->head = newNode;
                cache->tail = newNode;
            } else {
                // Cache is not empty
                cache->tail->next = newNode;
                newNode->prev = cache->tail;
                cache->tail = newNode;
            }
            break;
        case 2:  // LRU
            // Add the new node to the front of the cache
            newNode->next = cache->head;
            if (cache->head != NULL) {
                cache->head->prev = newNode;
            }
            cache->head = newNode;

            // If the cache was empty, update the tail
            if (cache->tail == NULL) {
                cache->tail = newNode;
            }
            break;
        case 3:  // Clock
            // Add the new node to the end of the cache
            if (cache->tail == NULL) {
                // Cache is empty
                cache->head = newNode;
                cache->tail = newNode;
            } else {
                // Cache is not empty
                cache->tail->next = newNode;
                newNode->prev = cache->tail;
                cache->tail = newNode;
            }
            break;
        default:
            fprintf(stderr, "Invalid eviction policy\n");
            exit(EXIT_FAILURE);
    }

    // Increment the count of items in the cache
    cache->count++;

    // Check if eviction is needed
    evictIfNeeded(cache, evictionPolicy);
}



// Function to process each item from stdin
void processInput(Cache* cache, int evictionPolicy) {
    char buffer[1024];
    // Read items from stdin until it's closed
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Remove newline character from the input
        size_t length = strlen(buffer);
        if (length > 0 && buffer[length - 1] == '\n') {
            buffer[length - 1] = '\0';
        }

        // Check if the item is in the cache
        if (isInCache(cache, buffer)) {
            printf("HIT\n");
        } else {
            printf("MISS\n");
            // Increment the appropriate miss counter
            if (cache->count <= cache->size) {
                // Compulsory miss
                cache->compulsoryMisses++;
            } else {
                // Capacity miss
                cache->capacityMisses++;
            }
            addToCache(cache, buffer, evictionPolicy);
        }
    }
}



int main(int argc, char* argv[]) {
    // Check command-line arguments
    if (argc != 4) {
        fprintf(stderr, "Usage: %s -N size <policy>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Parse cache size and eviction policy from command line
    int size = atoi(argv[2]);
    int evictionPolicy;
    if (strcmp(argv[3], "-F") == 0) {
        evictionPolicy = 1;  // FIFO
    } else if (strcmp(argv[3], "-L") == 0) {
        evictionPolicy = 2;  // LRU
    } else if (strcmp(argv[3], "-C") == 0) {
        evictionPolicy = 3;  // Clock
    } else {
        fprintf(stderr, "Invalid eviction policy\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the cache
    Cache cache;
    initializeCache(&cache, size);

    // Process input from stdin
    processInput(&cache, evictionPolicy);

    
    // Print summary line
    printf("%d %d\n", cache.compulsoryMisses, cache.capacityMisses);

    // Clean up and exit
    freeCache(&cache);

    return 0;
}

