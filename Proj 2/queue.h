#ifndef QUEUE_H
#define QUEUE_H

#include <stdatomic.h>

// Defines a item inside the queue.
typedef struct queue_item_t {
    struct queue_item_t* next;
    void** data;
} queue_item_t;

// Defines a circular buffer which acts as a FIFO queue.
typedef struct queue_t {
    atomic_uint size;
    queue_item_t* head;
} queue_t;

// Allocates and returns a new queue. Returns NULL if
// initialization failed.
queue_t* queue_init();

// Releases the queue resources.
void queue_dispose(queue_t* queue);

// Enqueues an item in the queue.
int queue_add(queue_t* queue, void* value);

// Dequeues an item from the head of the queue. Returns NULL if the queue is
// empty.
void* queue_remove(queue_t* queue);

// Returns, but does not remove, the head of the queue. Returns NULL if the
// queue is empty.
void* queue_peek(queue_t*);
#endif
