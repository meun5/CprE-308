#include <stdlib.h>
#include <stdio.h>

#include "queue.h"

int main() {
    queue_t* queue = queue_init();

    queue_add(queue, (void *) 1);
    queue_add(queue, (void *) 2);
    queue_add(queue, (void *) 3);

    printf("Queue Size: %u\n", queue->size);
    while (queue_peek(queue) != NULL) {
        printf("Queue Data: %d %u\n", (int) queue_remove(queue), queue->size);
    }

    queue_dispose(queue);

    return 0;
}
