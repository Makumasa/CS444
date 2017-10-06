#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "rand.h"

struct event {
        unsigned long val;
        unsigned long wait;
};

struct event_buffer {
        struct event *events;
        int count;
};

struct event_buffer *buffer;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void sig_handler(int signal)
{
        perror("Terminating...");
        pthread_mutex_lock(&mutex);
        free(buffer->events);
        free(buffer);
        exit(0);
}

void *producer(void *dummy)
{
        struct event e;
        e.val = rand_uint();
        e.wait = rand_uint_inclusive(2, 9);
        while (1) {
                pthread_mutex_lock(&mutex);
                if (buffer->count < 32) {
                        buffer->events[buffer->count++] = e;
                        printf("Added event %d to buffer.", e.val);
                        pthread_mutex_unlock(&mutex);
                        sleep(rand_uint_inclusive(3, 7));
                } else {
                        pthread_mutex_unlock(&mutex);
                        sleep(1);
                }
        }
}

void *consumer(void *dummy)
{
        while (1) {
                pthread_mutex_lock(&mutex);
                if (buffer->count > 0) {
                        struct event *e = &(buffer->events[--(buffer->count)]);
                        printf("Consumed event %d.", e->val);
                        int wait_time = e->wait;
                        pthread_mutex_unlock(&mutex);
                        sleep(wait_time);
                } else {
                        pthread_mutex_unlock(&mutex);
                        sleep(1);
                }
        }
}

int main(int argc, char **argv)
{
        /* Ignore Ctrl+C until buffer is allocated */
        struct sigaction action;
        action.sa_handler = SIG_IGN;
        sigfillset(&(action.sa_mask));
        sigaction(SIGINT, &action, NULL);

        buffer = malloc(sizeof(*buffer));
        buffer->events = calloc(32, sizeof(struct event));
        buffer->count = 0;

        /* Now that the buffer has been allocated, Ctrl+C will free and exit */
        action.sa_handler = &sig_handler;

        /* 
         * Use one producer and three consumers unless their number is specified
         * via command line
         */
        int num_producers = 1;
        int num_consumers = 3;
        if (argc > 2) {
                num_producers = atoi(argv[1]);
                num_consumers = atoi(argv[2]);
        }

        /* Create threads for producers */
        pthread_t pro_threads[num_producers];
        for (int i = 0; i < num_producers; ++i) {
                pthread_create(&pro_threads[i],
                               NULL,
                               producer,
                               NULL);
        }

        /* Create threads for consumers */
        pthread_t con_threads[num_consumers];
        for (int i = 0; i < num_consumers; ++i) {
                pthread_create(&con_threads[i],
                               NULL,
                               consumer,
                               NULL);
        }

        return 0;
}