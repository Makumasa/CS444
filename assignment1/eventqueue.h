#ifndef EVENTQUEUE_H_INCLUDED
#define EVENTQUEUE_H_INCLUDED

#include <stdbool.h>

struct event {
        unsigned long val;
        unsigned long wait;
};

struct event_queue {
        struct event *events;
        int capacity;
        int size;
        int front;
        int back;
};

struct event_queue *create_queue(int capacity);

void delete_queue(struct event_queue *queue);

bool is_empty(struct event_queue *queue);

bool is_full(struct event_queue *queue);

void enqueue(struct event_queue *queue, struct event item);

struct event dequeue(struct event_queue *queue);

#endif /* EVENTQUEUE_H_INCLUDED */