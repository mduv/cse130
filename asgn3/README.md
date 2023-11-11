# Reader-Writer Lock Implementation

## Overview
This repository contains an implementation of a reader-writer lock in C. The project aims to provide a thread-safe mechanism for managing concurrent access to shared resources.

## Design Decisions
- **PRIORITY Implementation:** The design includes support for different lock contention priorities, including READERS, WRITERS, and N-WAY.
- **Mutex and Condition Variable:** The implementation relies on pthread mutex and condition variable to ensure synchronization and avoid busy-waiting.
- **Dynamic Memory Allocation:** The lock structure is dynamically allocated and freed to enhance flexibility and memory management.

## High-Level Functions
### `rwlock_t* rwlock_new(PRIORITY p, uint32_t n);`
Dynamically allocates and initializes a new reader-writer lock with the specified priority (`p`) and, if using N-WAY priority, the specified `n` value.

### `void rwlock_delete(rwlock_t **rw);`
Deletes the reader-writer lock and frees all associated memory.

### `void reader_lock(rwlock_t *rw);`
Acquires the lock for reading.

### `void reader_unlock(rwlock_t *rw);`
Releases the lock after reading.

### `void writer_lock(rwlock_t *rw);`
Acquires the lock for writing.

### `void writer_unlock(rwlock_t *rw);`
Releases the lock after writing.

## Modules and Data Structures
- **rwlock_t Structure:** Represents the reader-writer lock and contains fields for managing the lock state, priority, and counters for readers and writers.

- **Mutex and Condition Variable:** Utilized for ensuring thread safety and synchronization.

## How Vague Requirements Are Implemented
- **PRIORITY Handling:** The implementation provides a flexible solution for handling different priorities, allowing the user to choose the desired behavior.

# Queue Implementation

This repository contains a simple and efficient queue implementation in C for Assignment 3.

## Overview

The `queue` module provides a flexible and dynamic queue data structure that can be easily integrated into various applications. It includes functionalities such as creating a new queue, deleting a queue, pushing elements onto the queue, and popping elements from the queue.

## Queue Structure

The queue is represented by the `queue_t` structure, defined in `queue.h`.

## High-Level Functions
### `queue_t *queue_new(int size)`
Dynamically allocates and initializes a new queue with the specified maximum size.

### void `queue_delete(queue_t **q)`
Deletes the queue and frees its memory. Assigns the passed-in pointer to NULL after deallocation.

### bool `queue_push(queue_t *q, void *elem)`
Pushes an element onto the queue. Returns a boolean indicating success or failure.

### bool `queue_pop(queue_t *q, void **elem)`
Pops an element from the queue. Returns a boolean indicating success or failure.


