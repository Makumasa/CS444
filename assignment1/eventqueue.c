#include <stdlib.h>
#include "eventqueue.h"

struct event_queue *create_queue(int capacity)
{
        struct event_queue *queue = malloc(sizeof(*queue));
        queue->events   = calloc(capacity, sizeof(struct event));
        queue->capacity = capacity;
        queue->size     = 0;
        queue->front    = 0;
        queue->back     = 0;

        return queue;
}

void delete_queue(struct event_queue *queue)
{
        free(queue->events);
        free(queue);
}

bool is_empty(struct event_queue *queue)
{
        return (queue->size == 0);
}

bool is_full(struct event_queue *queue) 
{
        return (queue->size >= (queue->capacity - 1));
}

void enqueue(struct event_queue *queue, struct event item)
{
        queue->events[queue->back] = item;

        if (queue->back < (queue->capacity - 1))
                (queue->back)++;
        else
                queue->back = 0;

        (queue->size)++;
}

struct event dequeue(struct event_queue *queue) {
        struct event e = queue->events[queue->front];

        if (queue->front < (queue->capacity - 1))
                (queue->front)++;
        else
                queue->front = 0;

        (queue->size)--;

        return e;
}
