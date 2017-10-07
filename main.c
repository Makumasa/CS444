#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "rand.h"
#include "eventqueue.h"

#define BUFFER_SIZE           32
#define DEFAULT_NUM_PRODUCERS 5
#define DEFAULT_NUM_CONSUMERS 3

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
struct event_queue *buffer;

void sig_handler(int signal)
{
        perror("\nTerminating...\n");
        pthread_mutex_lock(&mutex);
        delete_queue(buffer);
        exit(0);
}

void *producer(void *dummy)
{
        while (1) {
                struct event e;
                e.val = rand_uint();
                e.wait = rand_uint_inclusive(2, 9);
                pthread_mutex_lock(&mutex);
                if (!is_full(buffer)) {
                        enqueue(buffer, e);
                        printf("Added event %lu to buffer.\n", e.val);
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
                if (!is_empty(buffer)) {
                        struct event e = dequeue(buffer);
                        printf("Consumed event %lu.\n", e.val);
                        pthread_mutex_unlock(&mutex);
                        sleep(e.wait);
                } else {
                        pthread_mutex_unlock(&mutex);
                        sleep(1);
                }
        }
}

int main(int argc, char **argv)
{
        /* Ignore Ctrl+C until buffer is allocated */
        signal(SIGINT, SIG_IGN);

        /* Initialize random number generator */
        rand_init();

        /* Create the event buffer */
        buffer = create_queue(BUFFER_SIZE);

        /* Now that the buffer has been allocated, Ctrl+C will free and exit */
        signal(SIGINT, sig_handler);

        /* Use defaults unless their number is specified via command line */
        int num_producers = DEFAULT_NUM_PRODUCERS;
        int num_consumers = DEFAULT_NUM_CONSUMERS;
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

        /* Wait for interrupt signal */
        pause();

        return 0;
}