#include <pthread.h>
#include <stdint.h>
#include <limits.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "thread_pool.h"

void *pool_run(void *data) {
    pool_t *self = data;
    size_t backoff = 100;
    do {
        if (self->task_count > 0 && self->task_count < SSIZE_MAX) {
            printf("[%i:%i] ", self->id, self->task_count);
            backoff = 10;
            pthread_mutex_lock(&(self->jobs));
            pool_task_t *h = self->head;
            self->head = self->head->next;
            pthread_mutex_unlock(&(self->jobs));
            
            (h->task_exec)(h->task_str);
            free(h);

            pthread_mutex_lock(&(self->jobs));
            {
                self->task_count--;
            }
            pthread_mutex_unlock(&(self->jobs));
        } else {
            pthread_mutex_unlock(&(self->jobs));
            usleep(backoff);
            backoff += 5;
        }
    } while(1);
}

thread_pool_t *init_pools(size_t npools) {
    thread_pool_t *result = calloc(sizeof(thread_pool_t), 1);
    pool_t *pools = result->header = calloc(sizeof(pool_t), npools+2);

    for(int i=0;i<npools+2;i++) {
        pools[i].id = i;
        pthread_mutex_init(&pools[i].rank, NULL);
        pthread_mutex_init(&pools[i].jobs, NULL);
        if (i==0) {
            pools[i].task_count = -1;
            pools[i].higher = &pools[i];
        } else {
            pools[i].task_count = 0;
            pools[i].higher = &pools[i-1];
        }
        if (i==npools+1) {
            pools[i].task_count = SSIZE_MAX;
            pools[i].lower = &pools[i];
        } else {
            pools[i].lower = &pools[i+1];
        }
        pthread_create(&(pools[i].threadid), NULL, pool_run, &(pools[i]));
    }
    return result;
}

void shift_ranking(pool_t *pool) {
    size_t backoff;
restart:
    backoff = 10;
    do {
        pthread_mutex_lock(&(pool->rank));
        if (pthread_mutex_trylock(&(pool->higher->rank))) {
            pthread_mutex_unlock(&(pool->rank));
        } else if (pthread_mutex_trylock(&(pool->lower->rank))) {
            pthread_mutex_unlock(&(pool->rank));
            pthread_mutex_unlock(&(pool->higher->rank));
        } else {
            goto locked;
        }
        usleep(backoff);
        backoff *= 1.4;
    } while(1);
locked:
    
    if (pool->task_count < pool->higher->task_count) {
        if (pthread_mutex_trylock(&(pool->higher->higher->rank))) {
            pthread_mutex_unlock(&(pool->higher->rank));
            pthread_mutex_unlock(&(pool->lower->rank));
            pthread_mutex_unlock(&(pool->rank));
            goto restart;
        }
        pool_t *nh = pool->higher->higher;
        pool->higher->higher->lower = pool;
        pool->higher->higher = pool;
        pool->higher->lower = pool->lower;
        pool->lower->higher = pool->higher;
        pool->lower = pool->higher;
        pool->higher = nh;
        pthread_mutex_unlock(&(pool->higher->rank));
        pthread_mutex_unlock(&(pool->lower->rank));
        pthread_mutex_unlock(&(pool->lower->lower->rank));
        pthread_mutex_unlock(&(pool->lower->lower->rank));
        pthread_mutex_unlock(&(pool->rank));
        goto restart;
    }

    if (pool->task_count > pool->lower->task_count) {
        if (pthread_mutex_trylock(&(pool->lower->lower->rank))) {
            pthread_mutex_unlock(&(pool->higher->rank));
            pthread_mutex_unlock(&(pool->lower->rank));
            pthread_mutex_unlock(&(pool->rank));
            goto restart;
        }
        pool_t *nl = pool->lower->lower;
        pool->lower->lower->higher = pool;
        pool->lower->lower = pool;
        pool->lower->higher = pool->higher;
        pool->higher->lower = pool->lower;
        pool->higher = pool->lower;
        pool->lower = nl;
        pthread_mutex_unlock(&(pool->lower->rank));
        pthread_mutex_unlock(&(pool->higher->rank));
        pthread_mutex_unlock(&(pool->higher->higher->rank));
        pthread_mutex_unlock(&(pool->lower->lower->rank));
        pthread_mutex_unlock(&(pool->rank));
        goto restart;
    }
    pthread_mutex_unlock(&(pool->higher->rank));
    pthread_mutex_unlock(&(pool->lower->rank));
    pthread_mutex_unlock(&(pool->rank));
}

void add_job(pool_t *p, void (*exec)(void *), void *data) {
    pool_task_t *ptt = calloc(sizeof(pool_task_t), 1);
    ptt->task_exec = exec;
    ptt->task_str = data;

    if (!(p->head)) {
        p->head = ptt;
        p->tail = ptt;
    } else {
        p->tail->next = ptt;
        p->tail = ptt;
    }
}

void schedule(thread_pool_t *tp, void (*exec)(void *), void *data) {
    pool_t *pool = tp->header->lower;
    pthread_mutex_lock(&(pool->rank));
    pthread_mutex_lock(&(pool->jobs));
    add_job(pool, exec, data);
    pool->task_count++;
    pthread_mutex_unlock(&(pool->jobs));
    pthread_mutex_unlock(&(pool->rank));
    shift_ranking(pool);
}
