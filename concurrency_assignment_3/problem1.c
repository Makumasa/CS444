#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <semaphore.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>

#include "mt19937-64.h"

#define DEFAULT_NUM_THREADS 5

uint64_t resource = 0;
bool locked = false;
sem_t s;
pthread_mutex_t value_mutex = PTHREAD_MUTEX_INITIALIZER;

void sig_handler(int signal) {
    printf("\nTerminating...\n");
    exit(0);
}

void* foo(void* tid) {
    int id = *((int*)tid);
    while (1) {
        while (1) {
            pthread_mutex_lock(&value_mutex);
            if (!locked && (sem_trywait(&s) == 0)) {
                break;
            }
            pthread_mutex_unlock(&value_mutex);
            sleep(1);
        }

        int value;
        sem_getvalue(&s, &value);
        printf("thread %d has accessed the resource, semaphore value = %d\n",
               id, value);

        if (value) {
            pthread_mutex_unlock(&value_mutex);
            resource = genrand64_int64();
            sleep(genrand64_int64() % 3);
            pthread_mutex_lock(&value_mutex);
            sem_post(&s);
            sem_getvalue(&s, &value);
            printf("thread %d released the resource, semaphore value = %d\n",
                    id, value);
            pthread_mutex_unlock(&value_mutex);
        } else {
            locked = true;
            printf("thread %d locked the resource\n", id);
            pthread_mutex_unlock(&value_mutex);
            resource = genrand64_int64();
            sleep(genrand64_int64() % 3);
            pthread_mutex_lock(&value_mutex);
            sem_post(&s);
            sem_getvalue(&s, &value);
            printf("thread %d released the resource, semaphore value = %d\n",
                id, value);
            pthread_mutex_unlock(&value_mutex);
            do {
                sem_getvalue(&s, &value);
                sleep(1);
            } while (value < 3);
            locked = false;
            printf("thread %d unlocked the resource\n", id);
        }
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, sig_handler);
    sem_init(&s, 0, 3);
    init_genrand64(time(NULL));
    int num_threads = DEFAULT_NUM_THREADS;
    if (argc > 1) {
        num_threads = atoi(argv[1]);
    }
    pthread_t threads[num_threads];
    int tid[num_threads];

    for (int i = 0; i < num_threads; ++i) {
        tid[i] = i;
        pthread_create(&(threads[i]), NULL, foo, (void*)&tid[i]);
    }

    pause();

    return 0;
}
