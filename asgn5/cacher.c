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
    int referencedBit;      // For Clock
} Node;

// Add a history node structure
typedef struct HistoryNode {
    char* item;
    struct HistoryNode* next;
} HistoryNode;

// Define the cache structure
typedef struct {
    Node* head;             // Pointer to the head of the linked list
    Node* tail;             // Pointer to the tail of the linked list
    int size;               // Maximum size of the cache
    int count;              // Current number of items in the cache
    int compulsoryMisses;
    int capacityMisses;
    HistoryNode* historyHead;
    HistoryNode* historyTail;
} Cache;

// Function to initialize an empty cache
void initializeCache(Cache* cache, int size) {
    cache->head = NULL;
    cache->tail = NULL;
    cache->historyHead = NULL;
    cache->historyTail = NULL;
    cache->size = size;
    cache->count = 0;
}

// Function to print the items in the cache
void printCache(Cache* cache) {
    printf("Cache Contents: ");
    Node* current = cache->head;
    while (current != NULL) {
        printf("(%s, %d), ", current->item, current->referencedBit);
        current = current->next;
    }
    printf("\n");
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

    // Free the history nodes
    HistoryNode* historyCurrent = cache->historyHead;
    HistoryNode* historyNext;
    while (historyCurrent != NULL) {
        historyNext = historyCurrent->next;
        free(historyCurrent->item);
        free(historyCurrent);
        historyCurrent = historyNext;
    }

    // Reset the cache structure
    cache->head = NULL;
    cache->tail = NULL;
    cache->count = 0;
    cache->size = 0;
}

// Function to add an item to the history
void addToHistory(Cache* cache, const char* item) {
    // Allocate memory for the new history node
    HistoryNode* newHistoryNode = (HistoryNode*)malloc(sizeof(HistoryNode));
    if (newHistoryNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the item and copy it
    newHistoryNode->item = (char*)malloc(strlen(item) + 1);
    if (newHistoryNode->item == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(newHistoryNode);
        exit(EXIT_FAILURE);
    }
    strcpy(newHistoryNode->item, item);

    // Set default values for the new history node
    newHistoryNode->next = NULL;

    // Add the new history node to the end of the history
    if (cache->historyTail == NULL) {
        // History is empty
        cache->historyHead = newHistoryNode;
        cache->historyTail = newHistoryNode;
    } else {
        // History is not empty
        cache->historyTail->next = newHistoryNode;
        cache->historyTail = newHistoryNode;
    }
}

// Function to check if an item is in the history
int isInHistory(Cache* cache, const char* item) {
    HistoryNode* historyCurrent = cache->historyHead;
    while (historyCurrent != NULL) {
        if (strcmp(historyCurrent->item, item) == 0) {
            // Item found in the history
            return 1;
        }
        historyCurrent = historyCurrent->next;
    }
    // Item not found in the history
    return 0;
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

// Function to check if an item is in the cache and return the corresponding node
Node* isInCacheAndGetNode(Cache* cache, const char* item) {
    Node* current = cache->head;
    while (current != NULL) {
        if (strcmp(current->item, item) == 0) {
            // Item found in the cache, return the corresponding node
            return current;
        }
        current = current->next;
    }
    // Item not found in the cache
    return NULL;
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

// Helper function to swap two nodes in the cache order
void swapNodes(Cache* cache, Node* node1, Node* node2) {
    // Ensure both nodes are not NULL
    if (node1 == NULL || node2 == NULL) {
        return;
    }

    // Swap the nodes in the cache order
    Node* tempPrev = node1->prev;
    Node* tempNext = node1->next;

    node1->prev = node2->prev;
    node1->next = node2->next;

    if (node2->prev != NULL) {
        node2->prev->next = node1;
    } else {
        cache->head = node1;
    }

    if (node2->next != NULL) {
        node2->next->prev = node1;
    } else {
        cache->tail = node1;
    }

    node2->prev = tempPrev;
    node2->next = tempNext;

    if (tempPrev != NULL) {
        tempPrev->next = node2;
    }

    if (tempNext != NULL) {
        tempNext->prev = node2;
    }
}


// Function to evict an item using the Clock algorithm
void evictClock(Cache* cache, Node* newNode) {
    if (cache->head == NULL) {
        // Cache is empty, nothing to evict
        return;
    }

    Node* clockHand = cache->head;
    
    while (1) {
        if (clockHand->referencedBit == 0) {
            // Found an unreferenced item, evict it
            printf("ClockHand: %s\n", clockHand->item);
            if (cache->count > cache->size) {
                swapNodes(cache, clockHand, newNode);
            }

            // item to be evicted is clockhand, need to replace clockhand with the new item

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
void evictIfNeeded(Cache* cache, int evictionPolicy, Node* newNode) {
    // printf("cache count: %d\n", cache->count);
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
                evictClock(cache, newNode);
                break;
            default:
                fprintf(stderr, "Invalid eviction policy\n");
                exit(EXIT_FAILURE);
        }

        // Decrement the count of items in the cache
        cache->count--;
    }
}

// Function to update the cache order for LRU
void updateCacheLRU(Cache* cache, Node* accessedNode) {
    if (cache->tail == accessedNode) {
        return;
    }
    // Remove the accessed node from its current position
    if (accessedNode->prev != NULL) {
        accessedNode->prev->next = accessedNode->next;
    } else {
        // The accessed node is at the head
        cache->head = accessedNode->next;
    }

    if (accessedNode->next != NULL) {
        accessedNode->next->prev = accessedNode->prev;
    }

    // Update the tail to point to the accessed node
    accessedNode->prev = cache->tail;
    accessedNode->next = NULL;

    // Update the tail of the cache to the accessed node
    if (cache->tail != NULL) {
        cache->tail->next = accessedNode;
    }

    // Update the tail to the accessed node
    cache->tail = accessedNode;
}




// Function to add an item to the cache
void addToCache(Cache* cache, const char* item, int evictionPolicy) {
    // Check if the item is in the history
    int seenBefore = isInHistory(cache, item);

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

    if (evictionPolicy == 3) {
        newNode->referencedBit = 0;  // For Clock
    }

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



    // Increment the count of items in the cache
    cache->count++;

    // Check if the item is a compulsory miss or a capacity miss
    if (!seenBefore) {
        // Compulsory Miss
        cache->compulsoryMisses++;
    } else if (cache->count > cache->size) {
        // Capacity Miss
        cache->capacityMisses++;
    }

    // Add the item to the history
    addToHistory(cache, item);

    // Check if eviction is needed
    evictIfNeeded(cache, evictionPolicy, newNode);

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
        Node* accessedNode = isInCacheAndGetNode(cache, buffer);

        if (isInCache(cache, buffer)) {
            printf("HIT\n");
            // Update the cache order for LRU
            if (evictionPolicy == 2) {  // LRU
                updateCacheLRU(cache, accessedNode);
            }
            if (evictionPolicy == 3) {
                // updateCacheClock(cache, accessedNode);
                accessedNode->referencedBit = 1;
            }
            printCache(cache);
        } else {
            printf("MISS\n");
            addToCache(cache, buffer, evictionPolicy);
            printCache(cache);
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

