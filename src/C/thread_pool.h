#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct pool_task_s {
    struct pool_task_s *next;
    void (*task_exec)(void *);
    void *task_str;
} pool_task_t;

typedef struct pool_s {
    size_t id;
    pthread_t threadid;
    pthread_mutex_t jobs;
    pool_task_t *head;
    pool_task_t *tail;

    pthread_mutex_t rank;
    ssize_t task_count;
    struct pool_s *higher;
    struct pool_s *lower;
} pool_t;

typedef struct {
    pool_t *header;
} thread_pool_t;



thread_pool_t *init_pools(size_t);
void schedule(thread_pool_t *, void (*)(void *), void *);

#endif
