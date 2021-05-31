#define _GNU_SOURCE

#ifdef __APPLE__
#define _XOPEN_SOURCE
#endif

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

#if defined(_WIN32) && !defined(ENOBUFS)
#include <winsock.h>
#define ENOBUFS WSAENOBUFS
#endif

// Allocates and returns a new queue. Returns NULL if
// initialization failed.
queue_t* queue_init() {
    queue_t* queue = (queue_t*) malloc(sizeof(queue_t));
    if (!queue) {
        // In case of free(NULL), no operation is performed.
        free(queue);
        errno = ENOMEM;

        return NULL;
    }

    queue->size = 0;
    queue->head = NULL;

    return queue;
}

// Releases the queue resources.
void queue_dispose(queue_t* queue) {
    while (queue->head != NULL) {
        queue_item_t* temp = queue->head;
        queue->head = queue->head->next;
        free(temp);
    }

    queue->head = NULL;

    free(queue);
}

// Enqueues an item in the queue. Returns 0 if the add succeeded or -1 if it
// failed. If -1 is returned, errno will be set.
int queue_add(queue_t* queue, void* value) {
    queue_item_t* item = malloc(sizeof(queue_item_t*));

    if (item == NULL) {
        errno = ENOMEM;
        return -1;
    }

    item->data = value;
    item->next = NULL;

    if (queue->head != NULL) {
        queue_item_t* head = queue->head;
        while (head->next != NULL) {
            head = head->next;
        }

        head->next = item;
    } else {
        queue->head = item;
    }

    queue->size++;

    return 0;
}

// Dequeues an item from the head of the queue. Returns NULL if the queue is
// empty.
void* queue_remove(queue_t* queue) {
    void* value = NULL;

    if (queue->size > 0 && queue->head != NULL) {
        value = queue->head->data;

        queue_item_t* temp = queue->head;
        queue->head = queue->head->next;

        free(temp);

        queue->size--;
    }

    return value;
}

// Returns, but does not remove, the head of the queue. Returns NULL if the
// queue is empty.
void* queue_peek(queue_t* queue) {
    return queue->head != NULL ? queue->head->data : NULL;
}
