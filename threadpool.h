#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "condition.h"

typedef struct task
{
	void* (*run)(void* arg);
	void *arg;
	struct task *next;
}task_t;

typedef struct threadpool
{
	condition_t ready;
	task_t *first;
	task_t *last;
	int counter;
	int idle;
	int max_threads;
	int quit;
} threadpool_t;

void threadpool_init(threadpool* pool, int threads);

void threadpool_add_task(threadpool* pool, void* (*run)(void* arg), void *arg);

void threadpool_destory(threadpool* pool);

#endif